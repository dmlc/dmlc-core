/*!
 *  Copyright (c) 2015 by Contributors
 * \file text_parser.h
 * \brief iterator parser to parse text format
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_TEXT_PARSER_H_
#define DMLC_DATA_TEXT_PARSER_H_

#include <dmlc/data.h>
#include <dmlc/omp.h>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <algorithm>
#include "./row_block.h"
#include "./parser.h"

namespace dmlc {
namespace data {
/*!
 * \brief Text parser that parses the input lines
 * and returns rows in input data
 */
template <typename IndexType, typename DType = real_t>
class TextParserBase : public ParserImpl<IndexType, DType> {
 public:
  explicit TextParserBase(InputSplit *source,
                          int nthread)
      : bytes_read_(0), source_(source) {
    int maxthread = std::max(omp_get_num_procs() / 2 - 4, 1);
    nthread_ = std::min(maxthread, nthread);
  }
  virtual ~TextParserBase() {
    delete source_;
  }
  virtual void BeforeFirst(void) {
    source_->BeforeFirst();
  }
  virtual size_t BytesRead(void) const {
    return bytes_read_;
  }
  virtual bool ParseNext(std::vector<RowBlockContainer<IndexType, DType> > *data) {
    return FillData(data);
  }

 protected:
   /*!
    * \brief parse data into out
    * \param begin beginning of buffer
    * \param end end of buffer
    */
  virtual void ParseBlock(const char *begin, const char *end,
                          RowBlockContainer<IndexType, DType> *out) = 0;
   /*!
    * \brief read in next several blocks of data
    * \param data vector of data to be returned
    * \return true if the data is loaded, false if reach end
    */
  inline bool FillData(std::vector<RowBlockContainer<IndexType, DType>> *data);
   /*!
    * \brief start from bptr, go backward and find first endof line
    * \param bptr end position to go backward
    * \param begin the beginning position of buffer
    * \return position of first endof line going backward, returns begin if not found
    */
  static inline const char *BackFindEndLine(const char *bptr, const char *begin) {
     for (; bptr != begin; --bptr) {
       if (*bptr == '\n' || *bptr == '\r')
         return bptr;
     }
     return begin;
  }
  /*!
   * \brief Ignore UTF-8 BOM if present
   * \param begin reference to begin pointer
   * \param end reference to end pointer
   */
  static inline void IgnoreUTF8BOM(const char **begin, const char **end) {
    int count = 0;
    for (count = 0; *begin != *end && count < 3; count++, ++*begin) {
      if (!begin || !*begin)
        break;
      if (**begin != '\xEF' && count == 0)
        break;
      if (**begin != '\xBB' && count == 1)
        break;
      if (**begin != '\xBF' && count == 2)
        break;
    }
    if (count < 3)
      *begin -= count;
  }

 private:
  // nthread
  int nthread_;
  // number of bytes readed
  size_t bytes_read_;
  // source split that provides the data
  InputSplit *source_;
  // exception_ptr to hold exception thrown in OMP threads
  std::exception_ptr parser_exception_;
  // mutex for the exception_ptr
  std::mutex mutex_exception_;
};

// implementation
template <typename IndexType, typename DType>
inline bool TextParserBase<IndexType, DType>::FillData(
    std::vector<RowBlockContainer<IndexType, DType> > *data) {
  InputSplit::Blob chunk;
  if (!source_->NextChunk(&chunk)) return false;
  const int nthread = omp_get_max_threads();
  // reserve space for data
  data->resize(nthread);
  bytes_read_ += chunk.size;
  CHECK_NE(chunk.size, 0U);
  const char *head = reinterpret_cast<char *>(chunk.dptr);
#pragma omp parallel num_threads(nthread)
  {
    try {
      // threadid
      int tid = omp_get_thread_num();
      size_t nstep = (chunk.size + nthread - 1) / nthread;
      size_t sbegin = std::min(tid * nstep, chunk.size);
      size_t send = std::min((tid + 1) * nstep, chunk.size);
      const char *pbegin = BackFindEndLine(head + sbegin,
                                           head);
      const char *pend;
      if (tid + 1 == nthread) {
        pend = head + send;
      } else {
        pend = BackFindEndLine(head + send,
                               head);
      }
      ParseBlock(pbegin, pend, &(*data)[tid]);
    } catch (dmlc::Error& ex) {
      {
        std::lock_guard<std::mutex> lock(mutex_exception_);
        if (!parser_exception_) {
          parser_exception_ = std::current_exception();
        }
      }
    }
  }
  if (parser_exception_) {
    std::rethrow_exception(parser_exception_);
  }
  this->data_ptr_ = 0;
  return true;
}

}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_TEXT_PARSER_H_
