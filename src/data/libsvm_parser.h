/*!
 *  Copyright (c) 2015 by Contributors
 * \file libsvm_parser.h
 * \brief iterator parser to parse libsvm format
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_LIBSVM_PARSER_H_
#define DMLC_DATA_LIBSVM_PARSER_H_

#include <dmlc/data.h>
#include <cstring>
#include "./row_block.h"
#include "./text_parser.h"
#include "./strtonum.h"

namespace dmlc {
namespace data {
/*!
 * \brief Text parser that parses the input lines
 * and returns rows in input data
 */
template <typename IndexType, typename DType = real_t>
class LibSVMParser : public TextParserBase<IndexType> {
 public:
  explicit LibSVMParser(InputSplit *source,
                        int nthread)
      : TextParserBase<IndexType>(source, nthread) {}

 protected:
  virtual void ParseBlock(const char *begin,
                          const char *end,
                          RowBlockContainer<IndexType, DType> *out);
};

template <typename IndexType, typename DType>
void LibSVMParser<IndexType, DType>::
ParseBlock(const char *begin,
           const char *end,
           RowBlockContainer<IndexType, DType> *out) {
  out->Clear();
  const char * lbegin = begin;
  const char * lend = lbegin;
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
    // parse qid:id
    uint64_t qid;
    p = q;
    while (p != end && *p == ' ') ++p;
    if (p != lend && (strncmp(p, "qid:", 4) == 0))  {
      p += 4;
      qid = atoll(p);
      out->qid.push_back(qid);
      p = q;
    }
    // parse feature[:value]
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
