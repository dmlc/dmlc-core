/*!
 * Copyright (c) 2015 by Contributors
 * \file memory.h
 * \brief all things about memory like allocator
 */
#ifndef DMLC_MEMORY_H_
#define DMLC_MEMORY_H_
// this code depends on c++11
#if DMLC_USE_CXX11

#include <vector>

/*! \brief namespace for dmlc */
namespace dmlc {

/*!
 * \brief Object pool allocator. Object allocation may
 * reuse previously allocated one.
 * \author Tianqi Chen, Minjie Wang
 */
template <typename T, typename Guard = int>
class PoolAllocator {
 public:
  /*!\brief constructor*/
  PoolAllocator(): page_offset_(kPageSize) {}
  /*!\brief destructor*/
  ~PoolAllocator() {}
  /*!
   * \brief allocate a new object; may reuse an allocated object
   * \return the new allocated object
   */
  T* Alloc() {
    Guard g;
    if (!free_list_.empty()) {
      T* ret = free_list_.back();
      free_list_.pop_back();
      return ret;
    } else {
      return new T();
    }
  }
  /*!
   * \brief free an object. The pointer is in fact saves for next time.
   * \param p the object pointer
   */
  void Free(T* p) {
    Guard g;
    free_list_.push_back(obj);
  }

 private:
  std::vector<T*> free_list_;
};

}  // namespace dmlc
#endif  // DMLC_USE_CXX11
#endif  // DMLC_MEMORY_H_
