/*!
 *  Copyright (c) 2015 by Contributors
 * \file libsvm_parser.h
 * \brief iterator parser to parse libsvm format
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_LIBSVM_PARSER_H_
#define DMLC_DATA_LIBSVM_PARSER_H_

#include <dmlc/data.h>
#include <dmlc/omp.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include "./row_block.h"
#include "./parser.h"
#include "./strtonum.h"

namespace dmlc {
namespace data {
/*!
 * \brief libsvm parser that parses the input lines
 * and returns rows in input data
 */
template <typename IndexType>
class LibSVMParser : public ParserImpl<IndexType> {
 public:
  explicit LibSVMParser(InputSplit *source,
                        int nthread)
      : bytes_read_(0), source_(source) {
    int maxthread;
    #pragma omp parallel
    {
      maxthread = std::max(omp_get_num_procs() / 2 - 4, 1);
    }
    nthread_ = std::min(maxthread, nthread);
  }
  virtual ~LibSVMParser() {
    delete source_;
  }
  virtual void BeforeFirst(void) {
    source_->BeforeFirst();
  }
  virtual size_t BytesRead(void) const {
    return bytes_read_;
  }
  virtual bool ParseNext(std::vector<RowBlockContainer<IndexType> > *data) {
    return FillData(data);
  }

 protected:
  /*!
   * \brief read in next several blocks of data
   * \param data vector of data to be returned
   * \return true if the data is loaded, false if reach end
   */
  inline bool FillData(std::vector<RowBlockContainer<IndexType> > *data);
  /*!
   * \brief parse data into out
   * \param begin beginning of buffer
   * \param end end of buffer
   */
  inline void ParseBlock(char *begin,
                         char *end,
                         RowBlockContainer<IndexType> *out);
  /*!
   * \brief start from bptr, go backward and find first endof line
   * \param bptr end position to go backward
   * \param begin the beginning position of buffer
   * \return position of first endof line going backward
   */
  inline char* BackFindEndLine(char *bptr,
                               char *begin) {
    for (; bptr != begin; --bptr) {
      if (*bptr == '\n' || *bptr == '\r') return bptr;
    }
    return begin;
  }

 private:
  // nthread
  int nthread_;
  // number of bytes readed
  size_t bytes_read_;
  // source split that provides the data
  InputSplit *source_;
};

// implementation
template <typename IndexType>
inline bool LibSVMParser<IndexType>::
FillData(std::vector<RowBlockContainer<IndexType> > *data) {
  InputSplit::Blob chunk;
  if (!source_->NextChunk(&chunk)) return false;
  int nthread;
  #pragma omp parallel num_threads(nthread_)
  {
    nthread = omp_get_num_threads();
  }
  // reserve space for data
  data->resize(nthread);
  bytes_read_ += chunk.size;
  CHECK_NE(chunk.size, 0);
  char *head = reinterpret_cast<char*>(chunk.dptr);
  #pragma omp parallel num_threads(nthread_)
  {
    // threadid
    int tid = omp_get_thread_num();
    size_t nstep = (chunk.size + nthread - 1) / nthread;
    size_t sbegin = std::min(tid * nstep, chunk.size);
    size_t send = std::min((tid + 1) * nstep, chunk.size);
    char *pbegin = BackFindEndLine(head + sbegin, head);
    char *pend;
    if (tid + 1 == nthread) {
      pend = head + send;
    } else {
      pend = BackFindEndLine(head + send, head);
    }
    ParseBlock(pbegin, pend, &(*data)[tid]);
  }
  this->data_ptr_ = 0;
  return true;
}

template <typename IndexType>
inline void LibSVMParser<IndexType>::
ParseBlock(char *begin,
           char *end,
           RowBlockContainer<IndexType> *out) {
  out->Clear();
  char * lbegin = begin;
  char * lend = lbegin;
  while (lbegin != end) {
    // get line end
    lend = lbegin + 1;
    while (lend != end && *lend != '\n' && *lend != '\r') ++lend;
    // parse label[:weight]
    const char * p = lbegin;
    const char * q = NULL;
    real_t label;
    real_t weight;
    int r = ParsePair<real_t, real_t>(p, lend, &q, label, weight);
    if (r < 1) {
      // empty line
      lbegin = lend;
      continue;
    }
    if (r == 2) {
      // has weight
      out->weight.push_back(weight);
    }
    if (out->label.size() != 0) {
      out->offset.push_back(out->index.size());
    }
    out->label.push_back(label);
    // parse feature[:value]
    p = q;
    while (p != lend) {
      IndexType featureId;
      real_t value;
      int r = ParsePair<IndexType, real_t>(p, lend, &q, featureId, value);
      if (r < 1) {
        p = q;
        continue;
      }
      out->index.push_back(featureId);
      if (r == 2) {
        // has value
        out->value.push_back(value);
      }
      p = q;
    }
    // next line
    lbegin = lend;
  }
  if (out->label.size() != 0) {
    out->offset.push_back(out->index.size());
  }
  CHECK(out->label.size() + 1 == out->offset.size());
}

}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_LIBSVM_PARSER_H_
