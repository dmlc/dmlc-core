/*!
 *  Copyright (c) 2015 by Contributors
 * \file csv_parser.h
 * \brief iterator parser to parse csv format
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_CSV_PARSER_H_
#define DMLC_DATA_CSV_PARSER_H_

#include <dmlc/data.h>
#include <dmlc/parameter.h>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <set>
#include "./row_block.h"
#include "./text_parser.h"
#include "./strtonum.h"

namespace dmlc {
namespace data {

struct CSVParserParam : public Parameter<CSVParserParam> {
  std::string format;
  int label_column;
  int weight_column;
  int num_header_rows;
  std::vector<int> ignore_columns;
  // declare parameters
  DMLC_DECLARE_PARAMETER(CSVParserParam) {
    DMLC_DECLARE_FIELD(format).set_default("csv")
        .describe("File format.");
    DMLC_DECLARE_FIELD(label_column).set_default(-1)
        .describe("Column index that will put into label.");
    DMLC_DECLARE_FIELD(weight_column).set_default(-1)
        .describe("Column index that will put into weight.");
    DMLC_DECLARE_FIELD(num_header_rows).set_default(0).set_lower_bound(0)
        .describe("Number of header rows to ignore.");
    DMLC_DECLARE_FIELD(ignore_columns).set_default(std::vector<int>())
        .describe("Column indices that should be ignored.");
  }
};


/*!
 * \brief CSVParser, parses a dense csv format.
 *  Currently is a dummy implementation, when label column is not specified.
 *  All columns are treated as real dense data.
 *  label will be assigned to 0.
 *
 *  This should be extended in future to accept arguments of column types.
 */
template <typename IndexType>
class CSVParser : public TextParserBase<IndexType> {
 public:
  explicit CSVParser(InputSplit *source,
                     const std::map<std::string, std::string>& args,
                     int nthread)
      : TextParserBase<IndexType>(source, nthread) {
    param_.Init(args);
    CHECK_EQ(param_.format, "csv");
  }

 protected:
  virtual void ParseBlock(char *begin,
                          char *end,
                          RowBlockContainer<IndexType> *out);

 private:
  CSVParserParam param_;
};

template <typename IndexType>
void CSVParser<IndexType>::
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

  // Ignore the initial rows if asked
  for (int rows_to_ignore = param_.num_header_rows;
       rows_to_ignore > 0 && lbegin != end;
       --rows_to_ignore) {
    // get line end
    lend = lbegin + 1;
    while (lend != end && *lend != '\n' && *lend != '\r') ++lend;
    // skip empty lines
    while ((*lend == '\n' || *lend == '\r') && lend != end) ++lend;
    lbegin = lend;
  }

  while (lbegin != end) {
    // get line end
    lend = lbegin + 1;
    while (lend != end && *lend != '\n' && *lend != '\r') ++lend;

    char* p = lbegin;
    int column_index = 0;
    IndexType idx = 0;
    float label = 0.0f;

    while (p != lend) {
      // ignore column if asked to
      if (column_index < ignore_cols.size() && ignore_cols[column_index]) {
        // move p to the next element
        ++column_index;
        while (*p != ',' && p != lend) ++p;
        if (p != lend) ++p;
        continue;
      }
      // find the value at the current p
      char *endptr;
      float v = strtof(p, &endptr);
      p = endptr;
      // use column based on what it is
      if (column_index == param_.label_column) {
        label = v;
      } else if (column_index == param_.weight_column) {
        out->weight.push_back(v);
      } else {
        out->value.push_back(v);
        out->index.push_back(idx++);
      }
      // move p to the next element
      ++column_index;
      while (*p != ',' && p != lend) ++p;
      if (p != lend) ++p;
    }
    // skip empty line
    while ((*lend == '\n' || *lend == '\r') && lend != end) ++lend;
    lbegin = lend;
    out->label.push_back(label);
    out->offset.push_back(out->index.size());
  }
  CHECK(out->label.size() + 1 == out->offset.size());
  CHECK(out->weight.size() == 0 || out->weight.size() + 1 == out->offset.size());
}
}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_CSV_PARSER_H_
