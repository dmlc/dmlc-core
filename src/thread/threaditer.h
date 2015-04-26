/*!
 *  Copyright (c) 2015 by Contributors
 * \file threaditer.h
 * \brief the producer 
 * \author Tianqi Chen
 */
#ifndef DMLC_THREAD_THREADITER_H_
#define DMLC_THREAD_THREADITER_H_
// this code depends on c++11
#if defined(__GXX_EXPERIMENTAL_CXX0X) || __cplusplus >= 201103L
#include <dmlc/data.h>
#include <dmlc/logging.h>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace dmlc {
namespace thread {
/*!
 * \brief a iterator that was backed by a thread
 *  to pull data eagerly from a single producer into a bounded buffer
 *  the consumer can pull the data at its own rate
 *  
 * NOTE: thread concurrency cost time, make sure we store big blob of data in DType
 * 
 * Usage example:
 * \code
 * ThreadedIter<DType> iter;
 * iter.Init(&producer);
 * // the following code can be in parallel
 * DType *dptr;
 * while (iter.Next(&dptr)) {
 *   // do something on dptr
 *   // recycle the space
 *   iter.Recycle(&dptr);
 * }
 * \endcode
 * \tparam DType the type of data
 */
template<typename DType>
class ThreadedIter : public DataIter<DType> {
 public:
  /*!
   * \brief producer class interface
   *  that threaditer used as source to
   *  preduce the content
   */
  class Producer {
   public:
    // virtual destructor
    virtual ~Producer() {}
    /*! \brief reset the producer to beginning */
    virtual void BeforeFirst(void) {
      LOG(FATAL) << "not implemented";
    }
    /*!
     * \brief load the data content into DType,
     * the caller can pass in NULL or an existing address     
     * when inout_dptr is NULL:
     *    producer need to allocate a DType and fill the content
     * when inout_dptr is specified
     *    producer takes need to fill the content into address
     *    specified inout_dptr, or delete the one and create a new one
     *
     * \param inout_dptr used to pass in the data holder cell
     *        and return the address of the cell filled
     * \return true if there is next record, false if we reach the end     
     */
    virtual bool Next(DType **inout_dptr) = 0;
  };
  /*! \brief constructor */
  ThreadedIter(void)
      : producer_(NULL),
        own_producer_(false),
        producer_thread_(NULL),
        max_capacity_(8),
        nwait_consumer_(0),
        nwait_producer_(0),
        out_data_(NULL) {}
  /*! \brief destructor */
  virtual ~ThreadedIter(void) {
    if (producer_thread_ != NULL) {
      // lock the mutex
      std::unique_lock<std::mutex> lock(mutex_);
      // send destroy signal
      producer_sig_ = kDestroy;
      if (nwait_producer_ != 0) {
        producer_cond_.notify_one();
      }
      lock.unlock();
      producer_thread_->join();
      delete producer_thread_;
    }
    // end of critical region
    // now the slave thread should exit
    while (free_cells_.size() != 0) {
      delete free_cells_.front();
      free_cells_.pop();
    }
    while (queue_.size() != 0) {
      delete queue_.front();
      queue_.pop();
    }
    if (own_producer_ && producer_ != NULL) {
      delete producer_;
    }
    if (out_data_ != NULL) {
      delete out_data_;
    }  
  }
  /*!
   * \brief set maximum capacity of the queue 
   * \param max_capacity maximum capacity of the queue
   */
  inline void set_max_capacity(size_t max_capacity) {
    max_capacity_ = max_capacity;
  }
  /*!
   * \brief initialize the producer and start the thread
   *   can only be called once
   * \param producer pointer to the producer
   * \param pass_ownership whether pass the ownership to the iter
   *    if this is true, the threaditer will delete the producer
   *    when destructed
   */
  inline void Init(Producer *producer, bool pass_ownership = false);
  /*!
   * \brief get the next data, this function is threadsafe
   * \param out_dptr used to hold the pointer to the record
   *  after the function call, the caller takes ownership of the pointer
   *  the caller can call recycle to return ownership back to the threaditer
   *  so that the pointer can be re-used
   * \sa Recycle
   */
  inline bool Next(DType **out_dptr);
  /*!
   * \brief recycle the data cell, this function is threadsafe
   * the threaditer can reuse the data cell for future data loading
   * \param inout_dptr pointer to the dptr to recycle, after the function call
   *        the content of inout_dptr will be set to NULL
   */
  inline void Recycle(DType **inout_dptr);
  /*!
   * \brief adapt the iterator interface's Next 
   *  NOTE: the call to this function is not threadsafe
   *  use the other Next instead
   */
  virtual bool Next(void) {
    if (out_data_ != NULL) {
      this->Recycle(&out_data_);
    }
    if (Next(&out_data_)) {
      return true;
    } else {
      return false;
    }
  }
  /*!
   * \brief adapt the iterator interface's Value
   *  NOTE: the call to this function is not threadsafe
   *  use the other Next instead
   */
  virtual const DType &Value(void) const {
    CHECK(out_data_ != NULL) << "Calling Value at beginning or end?";
    return *out_data_;
  }
  /*! \brief set the iterator before first location */
  virtual void BeforeFirst(void) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (out_data_ != NULL) {
      free_cells_.push(out_data_);
      out_data_ = NULL;
    }
    if (producer_sig_ == kDestroy) {
      lock.unlock(); return;
    }
    producer_sig_ = kBeforeFirst;
    CHECK(!producer_sig_processed_);
    if (nwait_producer_ != 0) {
      producer_cond_.notify_one();
    }
    consumer_cond_.wait(lock, [this]() {
        return producer_sig_processed_;
      });
    while (queue_.size() != 0) {
      free_cells_.push(queue_.front());
      queue_.pop();
    }
    producer_sig_ = kProduce;
    produce_end_ = false;
    producer_sig_processed_ = false;
    lock.unlock();
    producer_cond_.notify_one();
  }
  
 private:
  /*! \brief signals send to producer */
  enum Signal {
    kProduce,
    kBeforeFirst,
    kDestroy
  };
  /*! \brief running thread of producer */
  inline void RunProducer(void);
  /*! \brief producer class */
  Producer *producer_;
  /*! \brief whether the threaditer owns the producer */
  bool own_producer_;
  /*! \brief signal to producer */
  Signal producer_sig_;
  /*! \brief whether the special signal other than kProduce is procssed */
  bool producer_sig_processed_;
  /*! \brief thread that runs the producer */
  std::thread *producer_thread_;
  /*! \brief whether produce ends */
  bool produce_end_;
  /*! \brief maximum queue size */
  size_t max_capacity_;
  /*! \brief internal mutex */
  std::mutex mutex_;
  /*! \brief number of consumer waiting */
  unsigned nwait_consumer_;
  /*! \brief number of consumer waiting */
  unsigned nwait_producer_;
  /*! \brief conditional variable for producer thread */
  std::condition_variable producer_cond_;
  /*! \brief conditional variable for consumer threads */  
  std::condition_variable consumer_cond_;
  /*! \brief the current output cell */
  DType *out_data_;
  /*! \brief internal queue of producer */
  std::queue<DType*> queue_;
  /*! \brief free cells that can be used */
  std::queue<DType*> free_cells_;
};

// implementation of functions
template<typename DType>
inline void ThreadedIter<DType>::
Init(Producer *producer, bool pass_ownership) {
  CHECK(producer_ == NULL) << "can only call Init once";
  producer_ = producer;
  own_producer_ = pass_ownership;  
  producer_sig_= kProduce;
  producer_sig_processed_ = false;
  produce_end_ = false;
  producer_thread_ = new std::thread([this]() {
      this->RunProducer();
    });
  CHECK(!producer_sig_processed_);
}

template<typename DType>
inline bool ThreadedIter<DType>::
Next(DType **out_dptr) {
  CHECK(producer_ != NULL) << "call Init first";
  if (producer_sig_ == kDestroy) return false;
  std::unique_lock<std::mutex> lock(mutex_);
  CHECK(producer_sig_ == kProduce)
      << "Make sure you call BeforeFirst not inconcurrent with Next!";
  ++nwait_consumer_;
  consumer_cond_.wait(lock, [this]() {
      return queue_.size() != 0 || produce_end_;
    });
  --nwait_consumer_;
  if (queue_.size() != 0) {
    *out_dptr = queue_.front();
    queue_.pop();
    bool notify = nwait_producer_ != 0 && !produce_end_;
    lock.unlock();
    if (notify) producer_cond_.notify_one();
    return true;
  } else  {
    CHECK(produce_end_);
    lock.unlock();
    return false;
  }
}

template<typename DType>
inline void ThreadedIter<DType>::Recycle(DType **inout_dptr) {
  std::unique_lock<std::mutex> lock(mutex_);
  free_cells_.push(*inout_dptr);
  *inout_dptr = NULL;
  bool notify = nwait_producer_ != 0 && !produce_end_;
  lock.unlock();
  if (notify) {
    printf("notify\n");
    producer_cond_.notify_one();
  }
}

template<typename DType>
inline void ThreadedIter<DType>::RunProducer(void) {
  while (true) {
    CHECK(!producer_sig_processed_);    
    std::unique_lock<std::mutex> lock(mutex_);
    ++nwait_producer_;
    producer_cond_.wait(lock, [this]() {
        if (producer_sig_ == kProduce) {
          return !produce_end_ &&
              (queue_.size() < max_capacity_ || free_cells_.size() != 0);
        } else {
          return true;
        }
      });
    --nwait_producer_;
    DType *cell = NULL;
    if (producer_sig_ == kProduce) {
      if (free_cells_.size() != 0) {
        cell = free_cells_.front();
        free_cells_.pop();
      }
      lock.unlock();
    } else if (producer_sig_ == kBeforeFirst) {
      // finish the job
      producer_->BeforeFirst();
      producer_sig_processed_ = true;
      consumer_cond_.notify_all();
      ++nwait_producer_;
      producer_cond_.wait(lock, [this]() {
          return producer_sig_ != kBeforeFirst;
        });
      --nwait_producer_;
      lock.unlock();
      continue;
    } else {
      // destroy the thread
      CHECK(producer_sig_ == kDestroy);
      producer_sig_processed_ = true;
      produce_end_ = true;
      lock.unlock();
      consumer_cond_.notify_all();
      return;
    }
    // now without lock    
    produce_end_ = !producer_->Next(&cell);
    CHECK(cell != NULL || produce_end_);
    // put things into queue
    lock.lock();
    bool notify = nwait_consumer_ != 0;
    if (!produce_end_) queue_.push(cell);
    lock.unlock();
    if (notify) consumer_cond_.notify_all();
  }
}
}  // namespace thread
}  // namespace dmlc
#endif  // C++11
#endif  // DMLC_THREADITER_H_
