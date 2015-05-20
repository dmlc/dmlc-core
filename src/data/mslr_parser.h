/*!
 *  Copyright (c) 2015 by Contributors
 * \file mslr_parser.h
 * \brief iterator parser to parse mslr format
 *        format: {label[:importance]} [featureid[:value]]*
 * \author Chuntao Hong
 */
#ifndef DMLC_DATA_MSLR_PARSER_H_
#define DMLC_DATA_MSLR_PARSER_H_

#include <vector>
#include <cstring>
#include <string>
#include <dmlc/data.h>
#include <dmlc/omp.h>
#include "./row_block.h"
#include "./parser.h"
#include "./strtonum.h"

namespace dmlc {
namespace data {
/*!
 * \brief mslr parser that parses the input lines
 * and returns rows in input data
 */
template <typename IndexType>
class MSLRParser : public Parser<IndexType> {
 public:
  explicit MSLRParser(InputSplit *source,
                        int nthread)
      : bytes_read_(0), source_(source) {
    int maxthread;
    #pragma omp parallel
    {
      maxthread = std::max(omp_get_num_procs() / 2 - 4, 1);
    }
    nthread_ = std::min(maxthread, nthread);
  }
  virtual ~MSLRParser() {
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
inline bool MSLRParser<IndexType>::
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
  this->data_ptr_ = 0;
  return true;
}

inline std::vector<std::string> SplitString(const std::string & str, const std::string & splitters) {
  using namespace std;
  vector<std::string> substrings;
  std::string::size_type start_pos = 0;
  while (start_pos < str.size())
  {
    std::string::size_type end = str.find_first_of(splitters, start_pos);
    if (end > start_pos)
      substrings.push_back(str.substr(start_pos, end - start_pos));
    if (end == std::string::npos)
      break;
    start_pos = end + 1;
  }
  return substrings;
};

inline std::string GetLine(char * begin, char * end, char ** nextp) {
  char * p = begin;
  while (p != end && *p != '\n') p++;
  if (p == end)
    *nextp = end;
  else
    *nextp = p + 1;
  return std::string(begin, p - begin);
}

template <typename IndexType>
inline void MSLRParser<IndexType>::
ParseBlock(char *begin,
           char *end,
           RowBlockContainer<IndexType> *out) {
  out->Clear();
  char *p = begin;
  while (p != end) {
    // get a line
    std::string line = GetLine(p, end, &p);
    auto tokens = SplitString(line, " \t");
    if (tokens.empty())
      continue;
    // parse label and importance
    auto li = SplitString(tokens[0], ":");
    if (out->label.size() != 0) {
      out->offset.push_back(out->index.size());
    }
    real_t label = static_cast<real_t>(atof(li[0].c_str()));
    label = (label == 1) ? 1 : 0;   // trim down to binary label
    out->label.push_back(label);
    if (li.size() > 1) {
      out->importance.push_back(static_cast<real_t>(atof(li[1].c_str())));
    }
    // parse features
    for (size_t i = 1; i < tokens.size(); i++) {
      auto & token = tokens[i];
      auto fv = SplitString(token, ":");
      out->index.push_back(strtoint<IndexType>(fv[0].c_str(), NULL, 10));
      if (fv.size() > 1) {
        out->value.push_back(static_cast<real_t>(atof(fv[1].c_str())));
      }
    }
  }
  if (out->label.size() != 0) {
    out->offset.push_back(out->index.size());
  }
  CHECK(out->label.size() + 1 == out->offset.size());
}

}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_LIBSVM_ITER_H_
