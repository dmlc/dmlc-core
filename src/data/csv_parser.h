/*!
 *  Copyright (c) 2015 by Contributors
 * \file csv_parser.h
 * \brief iterator parser to parse csv format
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_CSV_PARSER_H_
#define DMLC_DATA_CSV_PARSER_H_

#include <dmlc/data.h>
#include <cstring>
#include "./row_block.h"
#include "./text_parser.h"
#include "./strtonum.h"

namespace dmlc {
namespace data {
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
                        int nthread)
      : TextParserBase<IndexType>(source, nthread) {}

 protected:
  virtual void ParseBlock(char *begin,
                          char *end,
                          RowBlockContainer<IndexType> *out);
};

template <typename IndexType>
void CSVParser<IndexType>::
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

    char* p = lbegin;
    IndexType idx = 0;
    while (p != lend) {
      char *endptr;
      float v = strtof(p, &endptr);
      p = endptr;
      out->value.push_back(v);
      out->index.push_back(idx++);
      while (*p != ',' && p != lend) ++p;
      if (p != lend) ++p;
    }
    // skip empty line
    while ((*lend == '\n' || *lend == '\r') && lend != end) ++lend;
    lbegin = lend;
    out->label.push_back(0.0f);
    out->offset.push_back(out->index.size());
  }
  CHECK(out->label.size() + 1 == out->offset.size());
}
}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_CSV_PARSER_H_
