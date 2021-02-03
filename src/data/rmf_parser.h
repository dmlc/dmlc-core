/*!
 *  Copyright (c) 2015 by Contributors
 * \file rmf_parser.h
 * \brief iterator parser to parse recommendation multi-format feature format
 * \author Zhongyi Zheng 
 */
#ifndef DMLC_DATA_RMF_PARSER_H_
#define DMLC_DATA_RMF_PARSER_H_ 
#include <dmlc/data.h>
#include <dmlc/parameter.h>
#include <cstring>
#include "./row_block.h"
#include "./text_parser.h"

namespace dmlc {
namespace data {
// TODO check
namespace {
void split(const char *s, const char *end, char delim, std::vector<const char* > & out) {
  if (s == NULL) return;
  //char *p = const_cast<char *> (s);
  const char *p = s;
  out.push_back(p);
  for (size_t i = 1; i <= end - s; ++i)
    if (s[i - 1] == delim)
      out.push_back(p + i);
}
}

struct RMFParserParam : public Parameter<RMFParserParam> {
  std::string format;
  int multi_field_num;
  size_t label_width;
  // declare parameters
  DMLC_DECLARE_PARAMETER(RMFParserParam) {
    DMLC_DECLARE_FIELD(format).set_default("rmf")
        .describe("File format.");
    DMLC_DECLARE_FIELD(multi_field_num).set_default(1)
        .describe("The number of multi field feature.");
    DMLC_DECLARE_FIELD(label_width).set_default(1)
        .describe("The number of label.");
  }
};




/*!
 * \brief Text parser that parses the input lines
 * and returns rows in input data
 */
template <typename IndexType, typename DType = real_t>
class RMFParser : public TextParserBase<IndexType, DType> {
 public:
  explicit RMFParser(InputSplit *source,
                     const std::map<std::string, std::string>& args,
                        int nthread)
      : TextParserBase<IndexType>(source, nthread) {
    param_.Init(args);
    CHECK_GT(param_.multi_field_num, 1);
    CHECK_EQ(param_.format, "rmf");
  }

 protected:
  virtual void ParseBlock(const char *begin,
                          const char *end,
                          RowBlockContainer<IndexType, DType> *out);
 private:
  RMFParserParam param_;
// TODO check
  void ParseLibSVMUnitData(const char *lbegin,
                     const char *lend,
                     UnitBlockContainer<IndexType> *out) {
    const char * p = lbegin;
    const char * q = NULL;
    if (out->index.size() != 0) {
      out->offset.push_back(out->index.size());
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
  }
// TODO check
  void ParseCSVUnitData(const char *lbegin,
                     const char *lend,
                     UnitBlockContainer<IndexType> *out) {
    const char* p = lbegin;
    int column_index = 0;
    IndexType idx = 0;

    while (p != lend) {
      char *endptr;
      float v = strtof(p, &endptr);
      p = endptr;
      out->value.push_back(v);
      out->index.push_back(idx++);
      ++column_index;
      while (*p != ' ' && p != lend) ++p;
      if (p != lend) ++p;
    }
    out->offset.push_back(out->index.size());
  }

  void ParseCSVLabel(const char *lbegin,
                     const char *lend,
                     std::vector<real_t> &labels) {
    const char* p = lbegin;
    while (p != lend) {
      char *endptr;
      real_t v = strtof(p, &endptr);
      p = endptr;
      labels.push_back(v);
      while (*p != ' ' && p != lend) ++p;
      if (p != lend) ++p;
    }
  }
};

template <typename IndexType, typename DType>
void RMFParser<IndexType, DType>::
ParseBlock(const char *begin,
           const char *end,
           RowBlockContainer<IndexType, DType> *out) {
  out->Clear();
  out->label_width = param_.label_width;
  out->extra.resize(3 + param_.multi_field_num);
  const char * lbegin = begin;
  const char * lend = lbegin;
  while (lbegin != end) {
    // get line end
    lend = lbegin + 1;
    while (lend != end && *lend != '\n' && *lend != '\r') ++lend;
    if (lend == end) break;
    const char * p = lbegin;
    const char * q = nullptr;
    real_t label;
    real_t weight;
    std::vector<const char* > feats;
    split(p, lend, '\001', feats);
    if (feats.size() != 5) continue;
    ParseCSVLabel(feats[0], feats[1], out->label);
    // parse fieldid:feature:value
    ParseCSVUnitData(feats[1], feats[2], &(out->extra[0]));  // dense
    ParseCSVUnitData(feats[2], feats[3], &(out->extra[1]));  // cate
    ParseLibSVMUnitData(feats[4], lend, &(out->extra[2]));//sparse
    std::vector<const char* > multi_fields;
    split(feats[3], feats[4] - 2, ' ', multi_fields);
    if ( param_.multi_field_num != multi_fields.size())
      LOG(FATAL) << "The length of RMFParser's multi fields array isnot fixed " << param_.multi_field_num << " vs " << multi_fields.size(); 
    for (int i = 0; i < multi_fields.size() - 1; ++i)
      ParseLibSVMUnitData(multi_fields[i], multi_fields[i+1] - 1, &(out->extra[3 + i]));//multi field
    ParseLibSVMUnitData(multi_fields[multi_fields.size() - 1], feats[4] - 1, &(out->extra[2 + param_.multi_field_num]));//multi field
    // next line
    lbegin = lend;
  }
  if (out->label.size() != 0) {
    for (size_t i = 2; i < out->extra.size(); ++i) {
      out->extra[i].offset.push_back(out->extra[i].index.size());
      CHECK((out->label.size() / param_.label_width) + 1 == out->extra[i].offset.size());
    }
    out->offset.resize(1 + (out->label.size() / param_.label_width));
  }
}

}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_RMF_PARSER_H_
