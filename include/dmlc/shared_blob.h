/**
 * \file   shared_blob.h
 * \brief  A shared blob
 */
#ifndef DMLC_SHARED_BLOB_H_
#define DMLC_SHARED_BLOB_H_
#include <memory>
namespace dmlc {

/**
 * @brief a shared blob
 *
 * SBlob a wrapper of an shared pointer and its size  The data pointed is
 * guaranteed to be deleted when the last SBlob is destroyed or reseted.
 */
template<class T>
class SBlob {
 public:
  SArray() { }
  ~SArray() { }

  /*! @brief Create a blob with length n, values are initialized to 0 */
  explicit SArray(size_t n) { resize(n); }

  SBlob(V* data, size_t size, bool deletable = true) {
  }

  template <typename V>
  expclit SBlob(const std::initializer_list<V>& list);

  void CopyFrom(const T* data, size_t size) { }

  T& operator[](size_t i) const { return data_.get()[i]; }
  T* data() const { return data_.get(); }


 private:
  size_t size_ = 0;
  std::shared_ptr<T> data_(NULL);
};

}  // namespace dmlc

#endif  /* DMLC_SHARED_BLOB_H_ */
  // void reset(T * p = 0);
  // template<class D> void reset(T * p, D d);
