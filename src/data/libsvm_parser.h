/*!
 *  Copyright (c) 2015 by Contributors
 * \file libsvm_parser.h
 * \brief iterator parser to parse libsvm format
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_LIBSVM_PARSER_H_
#define DMLC_DATA_LIBSVM_PARSER_H_

#include <vector>
#include <cstring>
// #include <cctype>
#include <dmlc/data.h>
#include <dmlc/omp.h>
#include "./row_block.h"
#include "./parser.h"
#include "./strtonum.h"

namespace dmlc {
namespace data {
/*!
 * \brief libsvm parser that parses the input lines
 * and returns rows in input data
 */
class LibSVMParser : public Parser {
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
  virtual bool ParseNext(std::vector<RowBlockContainer<size_t> > *data) {
    return FillData(data);
  }
 protected:
  /*!
   * \brief read in next several blocks of data
   * \param data vector of data to be returned
   * \return true if the data is loaded, false if reach end
   */
  inline bool FillData(std::vector<RowBlockContainer<size_t> > *data);
  /*!
   * \brief parse data into out
   * \param begin beginning of buffer
   * \param end end of buffer
   */
  inline void ParseBlock(char *begin,
                         char *end,
                         RowBlockContainer<size_t> *out);
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
inline bool LibSVMParser::
FillData(std::vector<RowBlockContainer<size_t> > *data) {
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
  CHECK(chunk.size != 0);
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
  data_ptr_ = 0;
  return true;
}

inline void LibSVMParser::
ParseBlock(char *begin,
           char *end,
           RowBlockContainer<size_t> *out) {
  out->Clear();
  char *p = begin;
  while (p != end) {
    while (isspace(*p) && p != end) ++p;
    if (p == end) break;
    char *head = p;
    while (isdigit(*p) && p != end) ++p;
    if (*p == ':') {
      out->index.push_back(atol(head));
      out->value.push_back(static_cast<real_t>(atof(p + 1)));
    } else {
      if (out->label.size() != 0) {
        out->offset.push_back(out->index.size());
      }
      out->label.push_back(static_cast<real_t>(atof(head)));
    }
    while (!isspace(*p) && p != end) ++p;
  }
  if (out->label.size() != 0) {
    out->offset.push_back(out->index.size());
  }
  CHECK(out->label.size() + 1 == out->offset.size());
}

}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_LIBSVM_ITER_H_
