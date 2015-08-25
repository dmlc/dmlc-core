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
#include "dmlc/base.h"

namespace dmlc {

/*!
 * \brief Simple userspace spinlock implementation.
 */
class Spinlock {
 public:
  Spinlock() = default;
  ~Spinlock() = default;
  /*!
   * \brief Acquire lock.
   */
  inline void lock() noexcept;
  /*!
   * \brief Release lock.
   */
  inline void unlock() noexcept;

 private:
  std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
  /*!
   * \brief Disable copy and move.
   */
  DISALLOW_COPY_AND_ASSIGN(Spinlock);
};

/*!
 * \brief Cocurrent blocking queue.
 */
template <typename T>
class ConcurrentBlockingQueue {
 public:
  ConcurrentBlockingQueue();
  ~ConcurrentBlockingQueue() = default;
  /*!
   * \brief Push element into the queue.
   * \param e Element to push into.
   * \tparam E the element type
   *
   * It will copy or move the element into the queue, depending on the type of
   * the parameter.
   */
  template <typename E>
  void Push(E&& e);
  /*!
   * \brief Pop element from the queue.
   * \param rv Element popped.
   * \return On false, the queue is exiting.
   *
   * The element will be copied or moved into the object passed in.
   */
  bool Pop(T* rv);
  /*!
   * \brief Pop everything.
   * \return The queue.
   */
  std::list<T> PopAll();
  /*!
   * \brief Signal the queue for destruction.
   *
   * After calling this method, all blocking pop call to the queue will return
   * false.
   */
  void SignalForKill();
  /*!
   * \brief Get the size of the queue.
   * \return The size of the queue.
   */
  size_t Size();

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic<bool> exit_now_;
  std::list<T> queue_;
  /*!
   * \brief Disable copy and move.
   */
  DISALLOW_COPY_AND_ASSIGN(ConcurrentBlockingQueue);
};

inline void Spinlock::lock() noexcept {
  while (lock_.test_and_set(std::memory_order_acquire)) {
  }
}

inline void Spinlock::unlock() noexcept {
  lock_.clear(std::memory_order_release);
}

template <typename T>
ConcurrentBlockingQueue<T>::ConcurrentBlockingQueue() : exit_now_{false} {}

template <typename T>
template <typename E>
void ConcurrentBlockingQueue<T>::Push(E&& e) {
  static_assert(std::is_same<typename std::remove_cv<
                                 typename std::remove_reference<E>::type>::type,
                             T>::value,
                "Types must match.");
  std::lock_guard<std::mutex> lock{mutex_};
  queue_.emplace_back(std::forward<E>(e));
  if (queue_.size() == 1) {
    cv_.notify_all();
  }
}

template <typename T>
bool ConcurrentBlockingQueue<T>::Pop(T* rv) {
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

template <typename T>
std::list<T> ConcurrentBlockingQueue<T>::PopAll() {
  std::lock_guard<std::mutex> lock{mutex_};
  std::list<T> rv;
  rv.swap(queue_);
  return rv;
}

template <typename T>
void ConcurrentBlockingQueue<T>::SignalForKill() {
  std::unique_lock<std::mutex> lock{mutex_};
  exit_now_.store(true);
  cv_.notify_all();
}

template <typename T>
size_t ConcurrentBlockingQueue<T>::Size() {
  std::unique_lock<std::mutex> lock{mutex_};
  return queue_.size();
}

}  // namespace dmlc

#endif  // DMLC_USE_CXX11
#endif  // DMLC_CONCURRENCY_H_
