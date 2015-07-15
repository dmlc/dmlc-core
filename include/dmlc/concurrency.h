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

}  // namespace dmlc

#endif  // DMLC_USE_CXX11
#endif  // DMLC_CONCURRENCY_H_
