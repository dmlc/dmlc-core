/*!
 * Copyright (c) 2015 by Contributors
 * \file concurrency.h
 * \brief thread-safe data structures.
 * \author Yutian Li
 */
#ifndef DMLC_CONCURRENCY_H_
#define DMLC_CONCURRENCY_H_
// this code depends on c++11
#if DMLC_USE_CXX11
#include <atomic>
#include <list>
#include <mutex>
#include <condition_variable>

namespace dmlc {

/*!
 * \brief Simple userspace spinlock implementation.
 */
class Spinlock {
 public:
  Spinlock() = default;
  /*!
   * \brief Disable copy and move.
   */
  Spinlock(Spinlock const&) = delete;
  Spinlock(Spinlock&&) = delete;
  Spinlock& operator=(Spinlock const&) = delete;
  Spinlock& operator=(Spinlock&&) = delete;
  ~Spinlock() = default;
  /*!
   * \brief Acquire lock.
   */
  void lock() noexcept {
    while (lock_.test_and_set(std::memory_order_acquire)) {
    }
  }
  /*!
   * \brief Release lock.
   */
  void unlock() noexcept {
    lock_.clear(std::memory_order_release);
  }

 private:
  std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
};

/*!
 * \brief Cocurrent blocking queue.
 */
template<typename T>
class ConcurrentBlockingQueue {
 public:
  ConcurrentBlockingQueue() = default;
  /*!
   * \brief Disable copy and move.
   */
  ConcurrentBlockingQueue(ConcurrentBlockingQueue const&) = delete;
  ConcurrentBlockingQueue(ConcurrentBlockingQueue&&) = delete;
  ConcurrentBlockingQueue& operator=(ConcurrentBlockingQueue const&) = delete;
  ConcurrentBlockingQueue& operator=(ConcurrentBlockingQueue&&) = delete;
  ~ConcurrentBlockingQueue() = default;
  /*!
   * \brief Push element into the queue.
   * It will copy or move the element into the queue, depending on the type of the parameter.
   * \param e Element to push into.
   */
  template<typename E>
  void Push(E&& e) {
    static_assert(
      std::is_same<typename std::remove_cv<
        typename std::remove_reference<E>::type>::type, T>::value,
      "Types must match.");
    std::lock_guard<std::mutex> lock{mutex_};
    queue_.emplace_back(std::forward<E>(e));
    if (queue_.size() == 1) {
      cv_.notify_all();
    }
  }
  /*!
   * \brief Pop element from the queue.
   * The element will be copied or moved into the object passed in.
   * \param e Element popped.
   * \return Whether the queue is not empty afterwards.
   */
  bool Pop(T* rv) {
    std::unique_lock<std::mutex> lock{mutex_};
    while (queue_.empty() && !exit_now_.load()) {
      cv_.wait(lock);
    }
    if (!exit_now_.load()) {
      *rv = std::move(queue_.front());
      queue_.pop_front();
      return true;
    } else {
      return false;
    }
  }
  /*!
   * \brief Pop everything.
   * \return The queue.
   */
  std::list<T> PopAll() {
    std::lock_guard<std::mutex> lock{mutex_};
    std::list<T> rv;
    rv.swap(queue_);
    return rv;
  }
  /*!
   * \brief Signal the queue for destruction.
   * After calling this method, all blocking pop call to the queue will return false.
   */
  void SignalForKill() {
    std::unique_lock<std::mutex> lock{mutex_};
    exit_now_.store(true);
    cv_.notify_all();
  }
  /*!
   * \brief Get the size of the queue.
   * \return The size of the queue.
   */
  size_t Size() {
    std::unique_lock<std::mutex> lock{mutex_};
    return queue_.size();
  }

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic<bool> exit_now_{false};
  std::list<T> queue_;
};

}  // namespace dmlc

#endif  // DMLC_USE_CXX11
#endif  // DMLC_CONCURRENCY_H_
