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
#include <map>
#include <vector>
#include <string>
#include "./row_block.h"
#include "./text_parser.h"
#include "./strtonum.h"

namespace dmlc {
namespace data {

struct LibSVMParserParam : public Parameter<LibSVMParserParam> {
  std::string format;
  std::vector<int> ignore_columns;
  // declare parameters
  DMLC_DECLARE_PARAMETER(LibSVMParserParam) {
    DMLC_DECLARE_FIELD(format).set_default("libsvm")
        .describe("File format.");
    DMLC_DECLARE_FIELD(ignore_columns).set_default(std::vector<int>())
        .describe("Column indices that should be ignored.");
  }
};

/*!
 * \brief Text parser that parses the input lines
 * and returns rows in input data
 */
template <typename IndexType>
class LibSVMParser : public TextParserBase<IndexType> {
 public:
  explicit LibSVMParser(InputSplit *source,
                        const std::map<std::string, std::string>& args,
                        int nthread)
      : TextParserBase<IndexType>(source, nthread) {
    param_.Init(args);
    CHECK_EQ(param_.format, "libsvm");
  }

 protected:
  virtual void ParseBlock(char *begin,
                          char *end,
                          RowBlockContainer<IndexType> *out);

 private:
  LibSVMParserParam param_;
};

template <typename IndexType>
void LibSVMParser<IndexType>::
ParseBlock(char *begin,
           char *end,
           RowBlockContainer<IndexType> *out) {
  out->Clear();
  char * lbegin = begin;
  char * lend = lbegin;

  // convert to a dense vector for faster searching
  int max_ignored_col = 0;
  if (param_.ignore_columns.size() != 0) {
    max_ignored_col = 1 + *std::max_element(
      param_.ignore_columns.begin(), param_.ignore_columns.end());
  }
  std::vector<bool> ignore_cols(max_ignored_col, false);
  for (std::vector<int>::iterator it = param_.ignore_columns.begin();
       it != param_.ignore_columns.end(); ++it) {
    ignore_cols[*it] = true;
  }

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
      if (r < 1 || (featureId < ignore_cols.size() && ignore_cols[featureId])) {
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
