/*!
 *  Copyright (c) 2015 by Contributors
 * \file thread_input_split.h
 * \brief a threaded version of InputSplit with a prefetch thread
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_THREAD_INPUT_SPLIT_H_
#define DMLC_IO_THREAD_INPUT_SPLIT_H_
#include <dmlc/base.h>
// this code depends on c++11
#if DMLC_USE_CXX11
#include <dmlc/threadediter.h>
#include "./input_split_base.h"

namespace dmlc {
namespace io {
/*!
 * \brief a threaded version of InputSplit 
 *  wraps an InputSplitBase to use an thread to prefetch the data
 */
class ThreadedInputSplit : public InputSplit {
 public:
  /*!
   * \brief constructor
   * \param base an base object to define how to read data
   */
  explicit ThreadedInputSplit(InputSplitBase *base)
      : base_(base), tmp_chunk_(NULL) {
    iter_.set_max_capacity(8);
    // initalize the iterator
    iter_.Init([base](InputSplitBase::Chunk **dptr) {
        if (*dptr == NULL) {
          *dptr = new InputSplitBase::Chunk(InputSplitBase::kBufferSize);
        }
        return (*dptr)->Load(base);
      }, [base]() { base->BeforeFirst(); });
  }
  // destructor
  virtual ~ThreadedInputSplit(void) {
    iter_.Destroy();
    if (tmp_chunk_ != NULL) delete tmp_chunk_;
    delete base_;
  }
  virtual void BeforeFirst() {
    iter_.BeforeFirst();
  }
  // implement next record
  virtual bool NextRecord(Blob *out_rec) {
    if (tmp_chunk_ == NULL) {
      if (!iter_.Next(&tmp_chunk_)) return false;
    }
    while (!base_->NextRecord(out_rec, tmp_chunk_)) {
      iter_.Recycle(&tmp_chunk_);
      if (!iter_.Next(&tmp_chunk_)) return false;      
    }
    return true;
  }
  // implement next chunk
  virtual bool NextChunk(Blob *out_chunk) {
    if (tmp_chunk_ == NULL) {
      if (!iter_.Next(&tmp_chunk_)) return false;
    }
    while (!base_->NextChunk(out_chunk, tmp_chunk_)) {
      iter_.Recycle(&tmp_chunk_);
      if (!iter_.Next(&tmp_chunk_)) return false;      
    }
    return true;
  }  
  
 private:
  /*! \brief the place where we get the data */
  InputSplitBase *base_;
  /*! \brief backend thread iterator */
  ThreadedIter<InputSplitBase::Chunk> iter_;
  /*! \brief current chunk of data */
  InputSplitBase::Chunk *tmp_chunk_;
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_USE_CXX11
#endif  // DMLC_IO_THREAD_INPUT_SPLIT_H_
