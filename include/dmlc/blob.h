/*!
 * \file   blob.h
 * \brief  Blob is a simple structure that contains a length and a pointer to an
 * external array,
 */
#ifndef DMCL_BLOB_H_
#define DMLC_BLOB_H_
#include <string>
#include <vector>
#include <memory>
#include <string>
#include "logging.h"
#if DMLC_USE_EIGEN
#include "Eigen/src/Core/Map.h"
#include "Eigen/src/Core/Array.h"
#endif  // DMLC_USE_EIGEN

namespace dmlc {

/*!
 * \brief Blob, Binary Large OBject, is a simple structure containing a pointer
 * into some external storage and a size. The user of a Blob must ensure that
 * the blob is not used after the corresponding external storage has been
 * deallocated.
 *
 * \tparam T the date type
 */
template <typename T>
struct Blob {
  T* data;
  size_t size;

  /*! \brief Create an empty blob */
  Blob() : data(NULL), size(0) { }

  /*! \brief Create a blob from std::vector */
  Blob(std::vector<T>& v) : data(v.data()), size(v.size()) { }

  T operator[] (size_t n) const {
    CHECK_LT(n, size);
    return data[n];
  }

#if DMLC_USE_EIGEN
  typedef Eigen::Map<
    Eigen::Array<T, Eigen::Dynamic, 1> > EigenArrayMap;
  /*! \brief Return a size() by 1 Eigen3 Array */
  EigenArrayMap ToEigenArray() const {
    return EigenArrayMap(data, size);
  }

  typedef Eigen::Map<
    const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> > EigenMatrixMap;
  /*! \brief Return a size()/k by k Eigen3 Matrix */
  EigenMatrixMap ToEigenMatrix(int k = 1) const {
    CHECK_EQ(size % k, 0);
    return EigenMatrixMap(data, size / k, k);
  }
#endif  // DMLC_USE_EIGEN

  std::string ShortDebugString() {
    return std::string();
  }
};

/**
 * \brief The const version of Blob
 */
template <typename T>
class CBlob {
 public:
  const T* data;
  size_t size;

  /*! \brief Create an empty blob */
  CBlob() : data(NULL), size(0) { }

  /*! \brief Create a blob from std::vector */
  CBlob(const std::vector<T>& v) : data(v.data()), size(v.size()) { }

  T operator[] (size_t n) const {
    CHECK_LT(n, size);
    return data[n];
  }

  std::string ShortDebugString() {
    return std::string();
  }
#if DMLC_USE_EIGEN
  typedef Eigen::Map<
    Eigen::Array<T, Eigen::Dynamic, 1> > EigenArrayMap;
  /*! \brief Return a size() by 1 Eigen3 Array */
  EigenArrayMap ToEigenArray() const {
    return EigenArrayMap(data, size);
  }

  typedef Eigen::Map<
    const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> > EigenMatrixMap;
  /*! \brief Return a size()/k by k Eigen3 Matrix */
  EigenMatrixMap ToEigenMatrix(int k = 1) const {
    CHECK_EQ(size % k, 0);
    return EigenMatrixMap(data, size / k, k);
  }
#endif  // DMLC_USE_EIGEN

};

/**
 * @brief a shared blob
 *
 * SBlob a wrapper of an shared pointer and its size  The data pointed is
 * guaranteed to be deleted when the last SBlob is destroyed or reseted.
 */
template<class T>
class SBlob {
 public:
  SBlob() { }
  ~SBlob() { }

  /*! @brief Create a blob with length n, values are initialized to 0 */
  explicit SBlob(size_t n) { }

  SBlob(T* data, size_t size, bool deletable = true) {
  }

  template <typename V>
  SBlob(const std::initializer_list<V>& list) { }

  void CopyFrom(const T* data, size_t size) { }

  T& operator[](size_t i) const { return data_.get()[i]; }
  T* data() const { return data_.get(); }

size_t size() { return size_; }
  std::string ShortDebugString() {
    return std::string();
  }
 private:
  size_t size_ = 0;
  std::shared_ptr<T> data_;
};

// TODO support string? i.e. construct from string, and ToString
// TODO support == !=
// TODO slice itself Segment(int pos, int
// TODO a mutable slice?

}  // namespace dmlc
#endif  // DMLC_BLOB_H_
