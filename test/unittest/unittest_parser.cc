#include "../src/data/csv_parser.h"
#include "../src/data/libsvm_parser.h"
#include <cstdio>
#include <cstdlib>
#include <dmlc/io.h>
#include <gtest/gtest.h>

using namespace dmlc;
using namespace dmlc::data;

namespace parser_test {
template <typename IndexType, typename DType = real_t>
class CSVParserTest : public CSVParser<IndexType, DType> {
public:
  explicit CSVParserTest(InputSplit *source,
                         const std::map<std::string, std::string> &args,
                         int nthread)
      : CSVParser<IndexType, DType>(source, args, nthread) {}
  void CallParseBlock(char *begin, char *end,
                      RowBlockContainer<IndexType, DType> *out) {
    CSVParser<IndexType, DType>::ParseBlock(begin, end, out);
  }
};

template <typename IndexType, typename DType = real_t>
class LibSVMParserTest : public LibSVMParser<IndexType, DType> {
public:
  explicit LibSVMParserTest(InputSplit *source, int nthread)
      : LibSVMParser<IndexType, DType>(source, nthread) {}
  void CallParseBlock(char *begin, char *end,
                      RowBlockContainer<IndexType, DType> *out) {
    LibSVMParser<IndexType, DType>::ParseBlock(begin, end, out);
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

TEST(CSVParser, test_int32_parse) {
  using namespace parser_test;
  InputSplit *source = nullptr;
  const std::map<std::string, std::string> args;
  std::unique_ptr<CSVParserTest<unsigned, int32_t>> parser(
      new CSVParserTest<unsigned, int32_t>(source, args, 1));
  RowBlockContainer<unsigned, int32_t> *rctr = new RowBlockContainer<unsigned, int32_t>();
  std::string data = "20000000,20000001,20000002,20000003\n"
                     "20000004,20000005,20000006,20000007\n"
                     "20000008,20000009,20000010,20000011\n";
  char *out_data = const_cast<char *>(data.c_str());
  parser->CallParseBlock(out_data, out_data + data.size(), rctr);
  for (size_t i = 0; i < rctr->value.size(); i++) {
    CHECK((i+20000000) == rctr->value[i]);
  }
}

TEST(CSVParser, test_int64_parse) {
  using namespace parser_test;
  InputSplit *source = nullptr;
  const std::map<std::string, std::string> args;
  std::unique_ptr<CSVParserTest<unsigned, int64_t>> parser(
    new CSVParserTest<unsigned, int64_t>(source, args, 1));
  RowBlockContainer<unsigned, int64_t> *rctr = new RowBlockContainer<unsigned, int64_t>();
  std::string data = "2147483648,2147483649,2147483650,2147483651\n"
                     "2147483652,2147483653,2147483654,2147483655\n"
                     "2147483656,2147483657,2147483658,2147483659\n";
  char *out_data = const_cast<char *>(data.c_str());
  parser->CallParseBlock(out_data, out_data + data.size(), rctr);
  for (size_t i = 0; i < rctr->value.size(); i++) {
    CHECK((i+2147483648) == rctr->value[i]);
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

TEST(LibSVMParser, test_qid) {
  using namespace parser_test;
  InputSplit *source = nullptr;
  std::unique_ptr<LibSVMParserTest<unsigned>> parser(
      new LibSVMParserTest<unsigned>(source, 1));
  RowBlockContainer<unsigned>* rctr = new RowBlockContainer<unsigned>();
  std::string data = R"qid(3 qid:1 1:1 2:1 3:0 4:0.2 5:0
                           2 qid:1 1:0 2:0 3:1 4:0.1 5:1
                           1 qid:1 1:0 2:1 3:0 4:0.4 5:0
                           1 qid:1 1:0 2:0 3:1 4:0.3 5:0
                           1 qid:2 1:0 2:0 3:1 4:0.2 5:0
                           2 qid:2 1:1 2:0 3:1 4:0.4 5:0
                           1 qid:2 1:0 2:0 3:1 4:0.1 5:0
                           1 qid:2 1:0 2:0 3:1 4:0.2 5:0
                           2 qid:3 1:0 2:0 3:1 4:0.1 5:1
                           3 qid:3 1:1 2:1 3:0 4:0.3 5:0
                           4 qid:3 1:1 2:0 3:0 4:0.4 5:1
                           1 qid:3 1:0 2:1 3:1 4:0.5 5:0)qid";
  char* out_data = const_cast<char*>(data.c_str());
  parser->CallParseBlock(out_data, out_data + data.size(), rctr);
  const std::vector<size_t> expected_offset{
    0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60
  };
  const std::vector<real_t> expected_label{
    3, 2, 1, 1, 1, 2, 1, 1, 2, 3, 4, 1
  };
  const std::vector<uint64_t> expected_qid{
    1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3
  };
  const std::vector<unsigned> expected_index{
    1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5,
    1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5,
    1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5
  };
  const std::vector<real_t> expected_value{
    1, 1, 0, 0.2, 0, 0, 0, 1, 0.1, 1, 0, 1, 0, 0.4, 0, 0, 0, 1, 0.3, 0,
    0, 0, 1, 0.2, 0, 1, 0, 1, 0.4, 0, 0, 0, 1, 0.1, 0, 0, 0, 1, 0.2, 0,
    0, 0, 1, 0.1, 1, 1, 1, 0, 0.3, 0, 1, 0, 0, 0.4, 1, 0, 1, 1, 0.5, 0
  };
  CHECK(rctr->offset == expected_offset);
  CHECK(rctr->label == expected_label);
  CHECK(rctr->qid == expected_qid);
  CHECK(rctr->index == expected_index);
  CHECK(rctr->value == expected_value);
}
