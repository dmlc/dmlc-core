#include <gtest/gtest.h>

#include <dmlc/parameter.h>

struct TestParam : public dmlc::Parameter<TestParam> {
  float a_float;
  int a_int;
  std::string a_str;
  bool a_bool;
  int a_enum;
  std::vector<int> a_int_vector;
  // declare parameters in header file
  DMLC_DECLARE_PARAMETER(TestParam) {
    DMLC_DECLARE_FIELD(a_float).set_default(0.1f).describe("A float.");
    DMLC_DECLARE_FIELD(a_int).set_range(0, 10).set_default(1).describe("A int.");
    DMLC_DECLARE_FIELD(a_str).set_default("str").describe("A str.");
    DMLC_DECLARE_FIELD(a_bool).set_default(true).describe("Name of the net.");
    DMLC_DECLARE_FIELD(a_enum).add_enum("a", 1).add_enum("b", 2).set_default(1)
      .describe("A enum.");
    DMLC_DECLARE_FIELD(a_int_vector).set_default(std::vector<int>())
      .describe("A vector of ints.");
  }
};

DMLC_REGISTER_PARAMETER(TestParam);

TEST(Parameter, doc) {
  EXPECT_EQ(TestParam::__DOC__(),
    "a_float : float, optional, default=0.1\n"
    "    A float.\n"
    "a_int : int, optional, default='1'\n"
    "    A int.\n"
    "a_str : string, optional, default='str'\n"
    "    A str.\n"
    "a_bool : boolean, optional, default=True\n"
    "    Name of the net.\n"
    "a_enum : {'a', 'b'}, optional, default='a'\n"
    "    A enum.\n"
    "a_int_vector : vector<int>, optional, default=()\n"
    "    A vector of ints.\n");
}

TEST(Parameter, a_float) {
  TestParam param;
  std::map<std::string, std::string> kwargs;
  param.Init(kwargs);
  EXPECT_FLOAT_EQ(param.a_float, 0.1f);
  kwargs["a_float"] = "0.001";
  param.Init(kwargs);
  EXPECT_FLOAT_EQ(param.a_float, 0.001f);
}

TEST(Parameter, a_int) {
  TestParam param;
  std::map<std::string, std::string> kwargs;
  param.Init(kwargs);
  EXPECT_EQ(param.a_int, 1);
  kwargs["a_int"] = "9";
  param.Init(kwargs);
  EXPECT_EQ(param.a_int, 9);
  kwargs["a_int"] = "11";
  EXPECT_ANY_THROW(param.Init(kwargs));
}

TEST(Parameter, a_str) {
  TestParam param;
  std::map<std::string, std::string> kwargs;
  param.Init(kwargs);
  EXPECT_EQ(param.a_str, "str");
  kwargs["a_str"] = "abc";
  param.Init(kwargs);
  EXPECT_EQ(param.a_str, "abc");
}

TEST(Parameter, a_bool) {
  TestParam param;
  std::map<std::string, std::string> kwargs;
  param.Init(kwargs);
  EXPECT_EQ(param.a_bool, true);
  kwargs["a_bool"] = "false";
  param.Init(kwargs);
  EXPECT_EQ(param.a_bool, false);
  kwargs["a_bool"] = "TRUE";
  param.Init(kwargs);
  EXPECT_EQ(param.a_bool, true);
  kwargs["a_bool"] = "FaLsE";
  param.Init(kwargs);
  EXPECT_EQ(param.a_bool, false);
}

TEST(Parameter, a_enum) {
  TestParam param;
  std::map<std::string, std::string> kwargs;
  param.Init(kwargs);
  EXPECT_EQ(param.a_enum, 1);
  kwargs["a_enum"] = "b";
  param.Init(kwargs);
  EXPECT_EQ(param.a_enum, 2);
}

TEST(Parameter, a_int_vector) {
  TestParam param;
  std::map<std::string, std::string> kwargs;
  std::vector<int> expected = {1, 2, 3};
  param.Init(kwargs);
  EXPECT_EQ(param.a_int_vector.size(), 0);
  kwargs["a_int_vector"] = "10";
  param.Init(kwargs);
  ASSERT_EQ(param.a_int_vector.size(), 1);
  ASSERT_EQ(param.a_int_vector[0], 10);
  kwargs["a_int_vector"] = "(1,2,3)";
  param.Init(kwargs);
  EXPECT_EQ(param.a_int_vector, expected);
  kwargs["a_int_vector"] = " ( 1 , 2 , 3 ) ";
  param.Init(kwargs);
  EXPECT_EQ(param.a_int_vector, expected);
  kwargs["a_int_vector"] = "(10,)";
  param.Init(kwargs);
  EXPECT_EQ(param.a_int_vector.size(), 1);
  ASSERT_EQ(param.a_int_vector[0], 10);
  kwargs["a_int_vector"] = "(10)";
  param.Init(kwargs);
  EXPECT_EQ(param.a_int_vector.size(), 1);
  ASSERT_EQ(param.a_int_vector[0], 10);

  kwargs["a_int_vector"] = "1,2,3";
  EXPECT_ANY_THROW(param.Init(kwargs));
  kwargs["a_int_vector"] = "()";
  EXPECT_ANY_THROW(param.Init(kwargs));
  kwargs["a_int_vector"] = "";
  EXPECT_ANY_THROW(param.Init(kwargs));
}
