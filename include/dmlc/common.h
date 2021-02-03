/*!
 *  Copyright (c) 2015 by Contributors
 * \file common.h
 * \brief defines some common utility function.
 */
#ifndef DMLC_COMMON_H_
#define DMLC_COMMON_H_

#include <vector>
#include <string>
#include <sstream>
#include <mutex>
#include <utility>
#include "./logging.h"

namespace dmlc {
/*!
 * \brief Split a string by delimiter
 * \param s String to be splitted.
 * \param delim The delimiter.
 * \return a splitted vector of strings.
 */
inline std::vector<std::string> Split(const std::string& s, char delim) {
  std::string item;
  std::istringstream is(s);
  std::vector<std::string> ret;
  while (std::getline(is, item, delim)) {
    ret.push_back(item);
  }
  return ret;
}

/*!
 * \brief hash an object and combines the key with previous keys
 */
template<typename T>
inline size_t HashCombine(size_t key, const T& value) {
  std::hash<T> hash_func;
  return key ^ (hash_func(value) + 0x9e3779b9 + (key << 6) + (key >> 2));
}

/*!
 * \brief specialize for size_t
 */
template<>
inline size_t HashCombine<size_t>(size_t key, const size_t& value) {
  return key ^ (value + 0x9e3779b9 + (key << 6) + (key >> 2));
}

/*!
 * \brief OMP Exception class captures and rethrows exception from OMP blocks
 */
class OMPException {
 private:
  // exception_ptr member to store the exception
  std::exception_ptr omp_exception_;
  // mutex to be acquired during catch to set the exception_ptr
  std::mutex mutex_;

 public:
  /*!
   * \brief should be called in a catch clause to capture the exception
   */
  void CaptureException() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!omp_exception_) {
      omp_exception_ = std::current_exception();
    }
  }

  /*!
   * \brief should be called from the main thread to rethrow the exception
   */
  void Rethrow() {
    if (this->omp_exception_) std::rethrow_exception(this->omp_exception_);
  }
};

#if defined(_OPENMP)
#define OMP_INIT() dmlc::OMPException omp_exc;
#define OMP_BEGIN() try {
#define OMP_END()                \
  } catch (dmlc::Error &ex) {    \
    omp_exc.CaptureException();  \
  } catch (std::exception &ex) { \
    omp_exc.CaptureException();  \
  }
#define OMP_THROW() omp_exc.Rethrow();
#else
#define OMP_INIT()
#define OMP_BEGIN()
#define OMP_END()
#define OMP_THROW()
#endif  // _OPENMP

}  // namespace dmlc

#endif  // DMLC_COMMON_H_
