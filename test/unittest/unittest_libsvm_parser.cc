#include <gtest/gtest.h>
#include <iostream>
#include <fstream>

#include "../../src/data/libsvm_parser.h"

TEST(LibSVMParser, basics) {
  std::string tmp_file = std::tmpnam(nullptr);
  std::ofstream fo;
  fo.open(tmp_file);
  fo << "0 1:0.1 4:1\n";
  fo << "1 2:0.2\n";
  fo.close();
  std::unique_ptr<dmlc::Parser<unsigned> > parser(
      dmlc::Parser<unsigned>::Create((tmp_file).c_str(), 0, 1, "libsvm"));

  parser->BeforeFirst();
  ASSERT_TRUE(parser->Next());
  dmlc::RowBlock<unsigned> block = parser->Value();
  ASSERT_EQ(block.size, 2);
  EXPECT_FLOAT_EQ(block[0].label, 0);
  EXPECT_FLOAT_EQ(block[1].label, 1);
  EXPECT_FLOAT_EQ(block[0].weight, block[1].weight);
  EXPECT_FLOAT_EQ(block[0].weight, 1);

  ASSERT_EQ(block[0].length, 2);
  ASSERT_EQ(block[1].length, 1);
  EXPECT_EQ(block[0].get_index(0), 1);
  EXPECT_EQ(block[0].get_index(1), 4);
  EXPECT_EQ(block[1].get_index(0), 2);
  EXPECT_FLOAT_EQ(block[0].value[0], 0.1);
  EXPECT_FLOAT_EQ(block[0].value[1], 1);
  EXPECT_FLOAT_EQ(block[1].value[0], 0.2);

  std::remove(tmp_file.c_str());
}

TEST(LibSVMParser, weights) {
  std::string tmp_file = std::tmpnam(nullptr);
  std::ofstream fo;
  fo.open(tmp_file);
  fo << "0:2 1:0.1 4:1\n";
  fo << "1:1 2:0.2\n";
  fo.close();

  std::unique_ptr<dmlc::Parser<unsigned> > parser(
      dmlc::Parser<unsigned>::Create((tmp_file).c_str(), 0, 1, "libsvm"));

  parser->BeforeFirst();
  ASSERT_TRUE(parser->Next());
  dmlc::RowBlock<unsigned> block = parser->Value();
  ASSERT_EQ(block.size, 2);
  EXPECT_FLOAT_EQ(block[0].weight, 2);
  EXPECT_FLOAT_EQ(block[1].weight, 1);

  std::remove(tmp_file.c_str());
}

TEST(LibSVMParser, ignore_columns) {
  std::string tmp_file = std::tmpnam(nullptr);
  std::ofstream fo;
  fo.open(tmp_file);
  fo << "0:2 1:0.1 4:1\n";
  fo << "1:1 2:0.2\n";
  fo.close();

  std::unique_ptr<dmlc::Parser<unsigned> > parser(
      dmlc::Parser<unsigned>::Create(
        (tmp_file + "?ignore_columns=(1,2,3)").c_str(), 0, 1, "libsvm"));

  parser->BeforeFirst();
  ASSERT_TRUE(parser->Next());
  dmlc::RowBlock<unsigned> block = parser->Value();
  ASSERT_EQ(block.size, 2);
  ASSERT_EQ(block[0].length, 1);
  ASSERT_EQ(block[1].length, 0);

  std::unique_ptr<dmlc::Parser<unsigned> > parser2(
      dmlc::Parser<unsigned>::Create(
        (tmp_file + "?ignore_columns=(4,5,6)").c_str(), 0, 1, "libsvm"));

  parser2->BeforeFirst();
  ASSERT_TRUE(parser2->Next());
  dmlc::RowBlock<unsigned> block2 = parser2->Value();
  ASSERT_EQ(block2.size, 2);
  ASSERT_EQ(block2[0].length, 1);
  ASSERT_EQ(block2[1].length, 1);

  std::remove(tmp_file.c_str());
}
