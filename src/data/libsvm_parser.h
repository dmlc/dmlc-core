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
#include <cctype>
#include <dmlc/data.h>
#include <dmlc/omp.h>
#include "./row_block.h"
#include "./parser.h"

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

  inline bool isspace(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f');
  }
  inline bool isdigit(char c) {
    return (c >= '0' && c <= '9');
  }
  inline double atof (const char *p) {
    // Skip leading white space, if any. Not necessary
    // while (isspace(*p) ) ++ p;

    // Get sign, if any.
    double sign = 1.0;
    if (*p == '-') {
      sign = -1.0; ++ p;
    } else if (*p == '+') {
      ++ p;
    }

    // Get digits before decimal point or exponent, if any.
    double value;
    for (value = 0.0; isdigit(*p); ++p) {
      value = value * 10.0 + (*p - '0');
    }

    // Get digits after decimal point, if any.
    if (*p == '.') {
      double pow10 = 10.0;
      ++ p;
      while (isdigit(*p)) {
        value += (*p - '0') / pow10;
        pow10 *= 10.0;
        ++ p;
      }
    }
    // Handle exponent, if any.
    int frac = 0;
    double scale = 1.0;
    if ((*p == 'e') || (*p == 'E')) {
      unsigned int expon;
      // Get sign of exponent, if any.
      p += 1;
      if (*p == '-') {
        frac = 1;
        p += 1;
      } else if (*p == '+') {
        p += 1;
      }
      // Get digits of exponent, if any.
      for (expon = 0; isdigit(*p); p += 1) {
        expon = expon * 10 + (*p - '0');
      }
      if (expon > 308) expon = 308;
      // Calculate scaling factor.
      while (expon >= 50) { scale *= 1E50; expon -= 50; }
      while (expon >=  8) { scale *= 1E8;  expon -=  8; }
      while (expon >   0) { scale *= 10.0; expon -=  1; }
    }
    // Return signed and scaled floating point result.
    return sign * (frac ? (value / scale) : (value * scale));
  }
  inline long atol(const char* p) {
    // Skip the sign, if any
    if (*p == '+') ++ p;
    long value;
    for (value = 0; isdigit(*p); ++p) {
      value = value * 10 + (*p - '0');
    }
    return value;
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
      // out->index.push_back(1);
      // out->value.push_back(1.1);
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
