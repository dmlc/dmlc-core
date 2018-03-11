// Copyright by contributors
#ifndef DMLC_IO_SINGLE_THREADED_INPUT_SPLIT_H_
#define DMLC_IO_SINGLE_THREADED_INPUT_SPLIT_H_

#include <dmlc/base.h>
#if DMLC_ENABLE_STD_THREAD
#include "./input_split_base.h"
#include <algorithm>
#include <dmlc/threadediter.h>

namespace dmlc {
namespace io {
class SingleThreadedInputSplit : public InputSplit {
public:
  explicit SingleThreadedInputSplit(InputSplitBase *base,
                                    const size_t batch_size)
      : buffer_size_(InputSplitBase::kBufferSize), batch_size_(batch_size),
        base_(base), tmp_chunk_(NULL) {}
  bool NextProducer(InputSplitBase::Chunk **dptr) {
    if (*dptr == NULL) {
      *dptr = new InputSplitBase::Chunk(buffer_size_);
    }
    return base_->NextBatchEx(*dptr, batch_size_);
  }
  void BeforeFirstProducer() { base_->BeforeFirst(); }
  virtual ~SingleThreadedInputSplit(void) {
    delete tmp_chunk_;
    delete base_;
  }
  virtual void BeforeFirst() {
    BeforeFirstProducer();
    if (tmp_chunk_ != NULL) {
      tmp_chunk_ = NULL;
    }
  }
  virtual void HintChunkSize(size_t chunk_size) {
    buffer_size_ = std::max(chunk_size / sizeof(uint32_t), buffer_size_);
  }

  virtual bool NextRecord(Blob *out_rec) {
    if (tmp_chunk_ == NULL) {
      if (!NextProducer(&tmp_chunk_))
        return false;
    }
    while (!base_->ExtractNextRecord(out_rec, tmp_chunk_)) {
      tmp_chunk_ = NULL;
      if (!NextProducer(&tmp_chunk_))
        return false;
    }
    return true;
  }

  virtual bool NextChunk(Blob *out_chunk) {
    if (tmp_chunk_ == NULL) {
      if (!NextProducer(&tmp_chunk_))
        return false;
    }
    while (!base_->ExtractNextChunk(out_chunk, tmp_chunk_)) {
      tmp_chunk_ = NULL;
      if (!NextProducer(&tmp_chunk_))
        return false;
    }
    return true;
  }

  virtual size_t GetTotalSize(void) { return base_->GetTotalSize(); }

  virtual void ResetPartition(unsigned part_index, unsigned num_parts) {
    base_->ResetPartition(part_index, num_parts);
    this->BeforeFirst();
  }

private:
  size_t buffer_size_;
  size_t batch_size_;
  InputSplitBase *base_;
  ThreadedIter<InputSplitBase::Chunk> iter_;
  InputSplitBase::Chunk *tmp_chunk_;
};
} // namespace io
} // namespace dmlc

#endif
#endif
