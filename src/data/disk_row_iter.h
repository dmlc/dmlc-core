/*!
 *  Copyright (c) 2015 by Contributors
 * \file basic_row_iter.h
 * \brief row based iterator that
 *   caches things into disk and then load segments
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_DISK_ROW_ITER_H_
#define DMLC_DATA_DISK_ROW_ITER_H_

#include <algorithm>
#include <string>

#include <dmlc/data.h>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include <dmlc/threadediter.h>
#include <dmlc/timer.h>

#include "./libsvm_parser.h"
#include "./row_block.h"

#if DMLC_ENABLE_STD_THREAD
namespace dmlc {
namespace data {
/*!
 * \brief basic set of row iterators that provides
 * \tparam IndexType the type of index we are using
 */
template <typename IndexType, typename DType = real_t>
class DiskRowIter : public RowBlockIter<IndexType, DType> {
 public:
  // page size 64MB
  static const size_t kPageSize = 64UL << 20UL;
  /*!
   * \brief disk row iterator constructor
   * \param parser parser used to generate this

   */
  explicit DiskRowIter(Parser<IndexType, DType> *parser, const char *cache_file, bool reuse_cache)
      : cache_file_(cache_file), fi_(NULL) {
    if (reuse_cache) {
      if (!TryLoadCache()) {
        this->BuildCache(parser);
        CHECK(TryLoadCache()) << "failed to build cache file " << cache_file;
      }
    } else {
      this->BuildCache(parser);
      CHECK(TryLoadCache()) << "failed to build cache file " << cache_file;
    }
    delete parser;
  }
  virtual ~DiskRowIter(void) {
    iter_.Destroy();
    delete fi_;
  }
  virtual void BeforeFirst(void) {
    iter_.BeforeFirst();
  }
  virtual bool Next(void) {
    if (iter_.Next()) {
      row_ = iter_.Value().GetBlock();
      return true;
    } else {
      return false;
    }
  }
  virtual const RowBlock<IndexType, DType> &Value(void) const {
    return row_;
  }
  virtual size_t NumCol(void) const {
    return num_col_;
  }

 private:
  // file place
  std::string cache_file_;
  // input stream
  SeekStream *fi_;
  // maximum feature dimension
  size_t num_col_;
  // row block to store
  RowBlock<IndexType, DType> row_;
  // iterator
  ThreadedIter<RowBlockContainer<IndexType, DType>> iter_;
  // load disk cache file
  inline bool TryLoadCache(void);
  // build disk cache
  inline void BuildCache(Parser<IndexType, DType> *parser);
};

// build disk cache
template <typename IndexType, typename DType>
inline bool DiskRowIter<IndexType, DType>::TryLoadCache(void) {
  SeekStream *fi = SeekStream::CreateForRead(cache_file_.c_str(), true);
  if (fi == NULL) {
    return false;
  }
  this->fi_ = fi;
  iter_.Init(
      [fi](RowBlockContainer<IndexType, DType> **dptr) {
        if (*dptr == NULL) {
          *dptr = new RowBlockContainer<IndexType, DType>();
        }
        return (*dptr)->Load(fi);
      },
      [fi]() { fi->Seek(0); });
  return true;
}

template <typename IndexType, typename DType>
inline void DiskRowIter<IndexType, DType>::BuildCache(Parser<IndexType, DType> *parser) {
  Stream *fo = Stream::Create(cache_file_.c_str(), "w");
  // back end data
  RowBlockContainer<IndexType, DType> data;
  num_col_ = 0;
  double tstart = GetTime();
  while (parser->Next()) {
    data.Push(parser->Value());
    double tdiff = GetTime() - tstart;
    if (data.MemCostBytes() >= kPageSize) {
      size_t bytes_read = parser->BytesRead();
      bytes_read = bytes_read >> 20UL;
      LOG(INFO) << bytes_read << "MB read," << bytes_read / tdiff << " MB/sec";
      num_col_ = std::max(num_col_, static_cast<size_t>(data.max_index) + 1);
      data.Save(fo);
      data.Clear();
    }
  }
  if (data.Size() != 0) {
    num_col_ = std::max(num_col_, static_cast<size_t>(data.max_index) + 1);
    data.Save(fo);
  }
  delete fo;
  double tdiff = GetTime() - tstart;
  LOG(INFO) << "finish reading at %g MB/sec" << (parser->BytesRead() >> 20UL) / tdiff;
}
}  // namespace data
}  // namespace dmlc
#endif  // DMLC_USE_CXX11
#endif  // DMLC_DATA_DISK_ROW_ITER_H_
