#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

#include "../../src/data/csv_parser.h"

std::string CreateTempCSV() {
  std::string tmp_file = std::tmpnam(nullptr);
  std::ofstream fo;
  fo.open(tmp_file);
  fo << "0,0.1,100L,05\n";
  fo << "1,0.2,100U,10\n";
  fo.close();
  return tmp_file;
}

TEST(CSVParser, basics) {
  std::string tmp_file = CreateTempCSV();
  std::unique_ptr<dmlc::Parser<unsigned> > parser(
      dmlc::Parser<unsigned>::Create((tmp_file).c_str(), 0, 1, "csv"));

  parser->BeforeFirst();
  ASSERT_TRUE(parser->Next());
  dmlc::RowBlock<unsigned> block = parser->Value();
  ASSERT_EQ(block.size, 2);
  EXPECT_FLOAT_EQ(block[0].label, 0);
  EXPECT_FLOAT_EQ(block[1].label, 0);
  EXPECT_FLOAT_EQ(block[0].weight, 1);
  EXPECT_FLOAT_EQ(block[1].weight, 1);

  std::vector<dmlc::real_t> row1 = {0, 0.1, 100, 5};
  std::vector<dmlc::real_t> row2 = {1, 0.2, 100, 10};
  EXPECT_EQ(block[0].length, block[1].length);
  for (unsigned j = 0; j < block[0].length; ++j) {
    EXPECT_EQ(block[0].get_index(j), j);
    EXPECT_EQ(block[1].get_index(j), j);
    EXPECT_FLOAT_EQ(block[0].value[j], row1[j]);
    EXPECT_FLOAT_EQ(block[1].value[j], row2[j]);
  }

  std::remove(tmp_file.c_str());
}

TEST(CSVParser, with_url) {
  std::string tmp_file = CreateTempCSV();
  std::unique_ptr<dmlc::Parser<unsigned> > parser(
      dmlc::Parser<unsigned>::Create((tmp_file + "?format=csv").c_str(),
                                     0, 1, "auto"));

  parser->BeforeFirst();
  ASSERT_TRUE(parser->Next());
  dmlc::RowBlock<unsigned> block = parser->Value();
  ASSERT_EQ(block.size, 2);

  std::remove(tmp_file.c_str());
}

TEST(CSVParser, with_label) {
  std::string tmp_file = CreateTempCSV();
  std::unique_ptr<dmlc::Parser<unsigned> > parser(
      dmlc::Parser<unsigned>::Create((tmp_file + "?label_column=0").c_str(),
                                     0, 1, "csv"));

  parser->BeforeFirst();
  ASSERT_TRUE(parser->Next());
  dmlc::RowBlock<unsigned> block = parser->Value();
  ASSERT_EQ(block.size, 2);
  EXPECT_EQ(block[0].label, 0);
  EXPECT_EQ(block[1].label, 1);
  EXPECT_EQ(block[0].length, 3);
  EXPECT_EQ(block[0].length, block[1].length);
  EXPECT_EQ(block.weight, nullptr);
  EXPECT_FLOAT_EQ(block[0].weight, 1);
  EXPECT_FLOAT_EQ(block[1].weight, 1);

  std::remove(tmp_file.c_str());
}

TEST(CSVParser, with_weight) {
  std::string tmp_file = CreateTempCSV();
  std::unique_ptr<dmlc::Parser<unsigned> > parser(
      dmlc::Parser<unsigned>::Create((tmp_file + "?weight_column=1").c_str(),
                                     0, 1, "csv"));

  parser->BeforeFirst();
  ASSERT_TRUE(parser->Next());
  dmlc::RowBlock<unsigned> block = parser->Value();
  ASSERT_EQ(block.size, 2);
  EXPECT_FLOAT_EQ(block[0].weight, 0.1);
  EXPECT_FLOAT_EQ(block[1].weight, 0.2);
  EXPECT_EQ(block[0].length, 3);
  EXPECT_EQ(block[0].length, block[1].length);

  std::remove(tmp_file.c_str());
}
