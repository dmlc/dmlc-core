#include "../src/data/csv_parser.h"
#include <cstdio>
#include <cstdlib>
#include <dmlc/io.h>
#include <gtest/gtest.h>

using namespace dmlc;
using namespace dmlc::data;

namespace parser_test {
template <typename IndexType>
class CSVParserTest : public CSVParser<IndexType> {
public:
  explicit CSVParserTest(InputSplit *source,
                         const std::map<std::string, std::string> &args,
                         int nthread)
      : CSVParser<IndexType>(source, args, nthread) {}
  void CallParseBlock(char *begin, char *end,
                      RowBlockContainer<IndexType> *out) {
    CSVParser<IndexType>::ParseBlock(begin, end, out);
  }
};

}

TEST(CSVParser, test_ignore_bom) {
  using namespace parser_test;
  InputSplit *source = nullptr;
  const std::map<std::string, std::string> args;
  std::unique_ptr<CSVParserTest<unsigned>> parser(
      new CSVParserTest<unsigned>(source, args, 1));
  std::string data = "\xEF\xBB\xBF\x31\n\xEF\xBB\x32\n";
  char *out_data = (char *)data.c_str();
  RowBlockContainer<unsigned> *rctr = new RowBlockContainer<unsigned>();
  parser->CallParseBlock(out_data, out_data + data.size(), rctr);
  CHECK(rctr->value[0] == 1);
  CHECK(rctr->value[1] == 0);
  data = "\xEF\xBB\xBF\x31\n\xEF\xBB\xBF\x32\n";
  out_data = (char *)data.c_str();
  rctr = new RowBlockContainer<unsigned>();
  parser->CallParseBlock(out_data, out_data + data.size(), rctr);

  CHECK(rctr->value[0] == 1);
  CHECK(rctr->value[1] == 2);
}

TEST(CSVParser, test_standard_case) {
  using namespace parser_test;
  InputSplit *source = nullptr;
  const std::map<std::string, std::string> args;
  std::unique_ptr<CSVParserTest<unsigned>> parser(
      new CSVParserTest<unsigned>(source, args, 1));
  RowBlockContainer<unsigned> *rctr = new RowBlockContainer<unsigned>();
  std::string data = "0,1,2,3\n4,5,6,7\n8,9,10,11\n";
  char *out_data = const_cast<char *>(data.c_str());
  parser->CallParseBlock(out_data, out_data + data.size(), rctr);
  for (size_t i = 0; i < rctr->value.size(); i++) {
    CHECK(i == rctr->value[i]);
  }
}

TEST(CSVParser, test_different_newlines) {
  using namespace parser_test;
  InputSplit *source = nullptr;
  const std::map<std::string, std::string> args;
  std::unique_ptr<CSVParserTest<unsigned>> parser(
      new CSVParserTest<unsigned>(source, args, 1));
  RowBlockContainer<unsigned> *rctr = new RowBlockContainer<unsigned>();
  std::string data = "0,1,2,3\r\n4,5,6,7\r\n8,9,10,11\r\n";
  char *out_data = const_cast<char *>(data.c_str());
  parser->CallParseBlock(out_data, out_data + data.size(), rctr);
  for (size_t i = 0; i < rctr->value.size(); i++) {
    CHECK(i == rctr->value[i]);
  }
}

TEST(CSVParser, test_noeol) {
  using namespace parser_test;
  InputSplit *source = nullptr;
  const std::map<std::string, std::string> args;
  std::unique_ptr<CSVParserTest<unsigned>> parser(
      new CSVParserTest<unsigned>(source, args, 1));
  RowBlockContainer<unsigned> *rctr = new RowBlockContainer<unsigned>();
  std::string data = "0,1,2,3\r\n4,5,6,7\r\n8,9,10,11";
  char *out_data = const_cast<char *>(data.c_str());
  parser->CallParseBlock(out_data, out_data + data.size(), rctr);
  for (size_t i = 0; i < rctr->value.size(); i++) {
    CHECK(i == rctr->value[i]);
  }
}
