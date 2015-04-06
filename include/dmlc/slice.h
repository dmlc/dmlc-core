/*!
 * \file   slice.h
 * \brief  Slice is a simple structure that contains a length and a pointer to an
 * external array, which is motivated from leveldb
 */
#ifndef DMCL_SLICE_H_
#define DMLC_SLICE_H_
#include <string>
#include <vector>
#include "logging.h"
#if DMLC_USE_EIGEN
#include "Eigen/src/Core/Map.h"
#include "Eigen/src/Core/Array.h"
#endif  // DMLC_USE_EIGEN

namespace dmlc {

/*!
 * \brief Slice is a simple structure containing a pointer into some external
 * storage and a size. The user of a Slice must ensure that the slice is not
 * used after the corresponding external storage has been deallocated.
 *
 * \tparam T the date type
 */
template <typename T>
class Slice {
 public:
  /*! \brief Create an empty slice. */
  Slice() : data_(NULL), size_(0) { }

  /*! \brief Create a slice that refers to d[0,n-1].
  Slice(const T* d, size_t n) : data_(d), size_(n) { }

  /*! \brief Create a slice from std::vector */
  Slice(const std::vector<T>& v) : data_(v.data()), size_(v.size()) { }

  /*! \brief Return a pointer to the beginning of the referenced data */
  const char* data() const { return data_; }

  /*! \brief Return the length (in bytes) of the referenced data */
  size_t size() const { return size_; }

  /*! \brief Return true iff the length of the referenced data is zero */
  bool empty() const { return size_ == 0; }

  /*! \brief Return the ith byte in the referenced data.
   *  REQUIRES: n < size()
   */
  T operator[] (size_t n) const {
    CHECK_LT(n, size());
    return data_[n];
  }

  /*! \brief Change this slice to refer to an empty array */
  void clear() { data_ = NULL; size_ = 0; }

  Slice segment(int pos, int n) const {
    CHECK_GE(pos, 0); CHECK_LE(pos+n, size());
    return Slice(data() + pos, n);
  }

  /// eigen3 support
#if DMLC_USE_EIGEN
  typedef Eigen::Map<
    const Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic> > EigenArrayMap;
  /*!
   * \brief Return a size()/k by k Eigen3 Array
   */
  EigenArrayMap ToEigenArray(int k = 1) const {
    CHECK_EQ(size() % k, 0);
    return EigenArrayMap(data(), size() / k, k);
  }

  typedef Eigen::Map<
    const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> > EigenMatrixMap;
  /*!
   * \brief Return a size()/k by k Eigen3 Matrix
   */
  EigenMatrixMap ToEigenMatrix(int k = 1) const {
    CHECK_EQ(size() % k, 0);
    return EigenMatrixMap(data(), size() / k, k);
  }
#endif  // DMLC_USE_EIGEN

  ///TODO mshadow /

 private:
  const T* data_;
  size_t size_;

  // Intentionally copyable
};

// TODO support string? i.e. construct from string, and ToString
// TODO support == !=
// TODO slice itself Segment(int pos, int
// TODO a mutable slice?

}  // namespace dmlc


#endif  // DMLC_SLICE_H_
