#include <dmlc/json.h>
#include <dmlc/io.h>
#include <dmlc/memory_io.h>
#include <dmlc/concurrentqueue.h>
#include <dmlc/blockingconcurrentqueue.h>
#include <gtest/gtest.h>
#include <mutex>
#include <condition_variable>

/*!
 * \brief Simple manually-signalled event gate which remains open
 */
class SimpleEvent {
 public:
  SimpleEvent()
    : signaled_(false) {}
  void wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!signaled_) {
      condition_variable_.wait(lock);
    }
  }
  void signal() {
    signaled_ = true;
    std::unique_lock<std::mutex> lk(mutex_);
    condition_variable_.notify_all();
  }

  /*! \brief Signal event upon destruction, even for exceptions (RAII) */
  struct SetReadyOnDestroy {
    explicit inline SetReadyOnDestroy(std::shared_ptr<SimpleEvent> *event)
      : event_(*event) {
    }
    inline ~SetReadyOnDestroy() {
      if (event_) {
        event_->signal();
      }
    }
    std::shared_ptr<SimpleEvent>  event_;
  };

 private:
  std::mutex              mutex_;
  std::condition_variable condition_variable_;
  std::atomic<bool>       signaled_;
};

/*!
 * \brief Simple thread lifecycle management
 */
class ThreadGroup {
 public:
  ~ThreadGroup() {
    WaitForAll();
  }
  size_t Count() const {
    std::unique_lock<std::mutex> lk(cs_threads_);
    return threads_.size();
  }
  void WaitForAll() {
    while(Count()) {
      std::shared_ptr<std::thread> thrd(nullptr);
      do {
        std::unique_lock<std::mutex> lk(cs_threads_);
        if (!threads_.empty()) {
          thrd = *threads_.begin();
          threads_.erase(thrd);
        }
      } while (false);
      if(thrd) {
        if(thrd->joinable()) {
          thrd->join();
        }
      }
    }
  }
  template<typename StartFunction, typename ...Args>
  void Start(StartFunction start_function, Args ...args) {
    std::unique_lock<std::mutex> lk(cs_threads_);
    std::shared_ptr<std::thread> thrd = std::make_shared<std::thread>(start_function, args...);
    threads_.insert(thrd);
  }
 private:
  mutable std::mutex                               cs_threads_;
  std::unordered_set<std::shared_ptr<std::thread>> threads_;
};

template<typename TQueue>
struct LFQThreadData {
  LFQThreadData() : count_(0) {}
  std::atomic<size_t> count_;
  std::shared_ptr<TQueue> q_ = std::make_shared<TQueue>();
  std::shared_ptr<SimpleEvent> ready_ = std::make_shared<SimpleEvent>();
  std::mutex cs_map_;
  std::set<int> thread_map_;
};

template<typename TQueue>
static void PushThread(const int id, std::shared_ptr<LFQThreadData<TQueue>> data) {
  ++data->count_;
  data->ready_->wait();
  data->q_->enqueue(id);
  std::unique_lock<std::mutex> lk(data->cs_map_);
  data->thread_map_.erase(id);
}

template<typename TQueue>
static void PullThread(const int id, std::shared_ptr<LFQThreadData<TQueue>> data) {
  ++data->count_;
  data->ready_->wait();
  int val;
  GTEST_ASSERT_EQ(data->q_->try_dequeue(val), true);
  std::unique_lock<std::mutex> lk(data->cs_map_);
  data->thread_map_.erase(id);
}

template<typename TQueue>
static void BlockingPullThread(const int id, std::shared_ptr<LFQThreadData<TQueue>> data) {
  ++data->count_;
  data->ready_->wait();
  int val;
  data->q_->wait_dequeue(val);
  std::unique_lock<std::mutex> lk(data->cs_map_);
  data->thread_map_.erase(id);
}

TEST(Lockfree, ConcurrentQueue) {
  ThreadGroup threads;
  const size_t ITEM_COUNT = 100;
  auto data = std::make_shared<LFQThreadData<dmlc::moodycamel::ConcurrentQueue<int>>>();
  for(size_t x = 0; x < ITEM_COUNT; ++x) {
    std::unique_lock<std::mutex> lk(data->cs_map_);
    data->thread_map_.insert(x);
    threads.Start(PushThread<dmlc::moodycamel::ConcurrentQueue<int>>, x, data);
  }
  while(data->count_ < ITEM_COUNT) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  data->ready_->signal();
  size_t remaining = ITEM_COUNT;
  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::unique_lock<std::mutex> lk(data->cs_map_);
    remaining = data->thread_map_.size();
  } while (remaining);

  size_t count = data->q_->size_approx();
  GTEST_ASSERT_EQ(count, ITEM_COUNT);

  threads.WaitForAll();
  GTEST_ASSERT_EQ(threads.Count(), 0UL);

  for(size_t x = 0; x < ITEM_COUNT; ++x) {
    std::unique_lock<std::mutex> lk(data->cs_map_);
    data->thread_map_.insert(x);
    threads.Start(PullThread<dmlc::moodycamel::ConcurrentQueue<int>>, x, data);
  }
  data->ready_->signal();
  threads.WaitForAll();
  GTEST_ASSERT_EQ(threads.Count(), 0UL);

  count = data->q_->size_approx();
  GTEST_ASSERT_EQ(count, 0UL);
}

TEST(Lockfree, BlockingConcurrentQueue) {

  using BlockingQueue = dmlc::moodycamel::BlockingConcurrentQueue<
    int, dmlc::moodycamel::ConcurrentQueueDefaultTraits>;

  ThreadGroup threads;
  const size_t ITEM_COUNT = 100;
  auto data = std::make_shared<LFQThreadData<BlockingQueue>>();
  for(size_t x = 0; x < ITEM_COUNT; ++x) {
    std::unique_lock<std::mutex> lk(data->cs_map_);
    data->thread_map_.insert(x);
    threads.Start(PushThread<BlockingQueue>, x, data);
  }
  while(data->count_ < ITEM_COUNT) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  data->ready_->signal();
  size_t remaining = ITEM_COUNT;
  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::unique_lock<std::mutex> lk(data->cs_map_);
    remaining = data->thread_map_.size();
  } while (remaining);

  size_t count = data->q_->size_approx();
  GTEST_ASSERT_EQ(count, ITEM_COUNT);

  threads.WaitForAll();
  GTEST_ASSERT_EQ(threads.Count(), 0UL);

  for(size_t x = 0; x < ITEM_COUNT; ++x) {
    std::unique_lock<std::mutex> lk(data->cs_map_);
    data->thread_map_.insert(x);
    threads.Start(BlockingPullThread<BlockingQueue>, x, data);
  }
  data->ready_->signal();
  threads.WaitForAll();
  GTEST_ASSERT_EQ(threads.Count(), 0UL);

  count = data->q_->size_approx();
  GTEST_ASSERT_EQ(count, 0UL);
}

