// Copyright by Contributors

#include <iostream>
#include <vector>

#include <dmlc/optional.h>
#include <dmlc/parameter.h>

#include <gtest/gtest.h>

TEST(Optional, constructors) {
  dmlc::optional<int> o1;
  CHECK(!o1);

  dmlc::optional<int> o2 = dmlc::nullopt;
  CHECK(!o2);

  dmlc::optional<int> o3 = 42;
  CHECK_EQ(*o3, 42);

  dmlc::optional<int> o4 = o3;
  CHECK_EQ(*o4, 42);

  dmlc::optional<int> o5 = o1;
  CHECK(!o5);

  dmlc::optional<int> o6 = std::move(o3);
  CHECK_EQ(*o6, 42);

  dmlc::optional<int> o7 = 42;
  CHECK_EQ(*o7, 42);

  dmlc::optional<int> o8 = o7;
  CHECK_EQ(*o8, 42);

  dmlc::optional<int> o9 = std::move(o7);
  CHECK_EQ(*o9, 42);

  dmlc::optional<int> o10, o11 = 1, o12 = o11;
  dmlc::optional<std::string> o13(dmlc::in_place, {'a', 'b', 'c'});
  CHECK_EQ(*o11, 1);
  CHECK_EQ(*o12, 1);
  CHECK_EQ(*o13, "abc");

  dmlc::optional<std::string> o14(dmlc::in_place, 3, 'A');
  CHECK_EQ(*o14, "AAA");
}

TEST(Optional, in_place) {
  dmlc::optional<int> o1{dmlc::in_place};
  CHECK(*o1 == 0);

  dmlc::optional<std::vector<int>> o2(dmlc::in_place, {0, 1});
  CHECK((*o2)[0] == 0);
  CHECK((*o2)[1] == 1);

  dmlc::optional<std::tuple<int, int>> o3(dmlc::in_place, 0, 1);
  CHECK(std::get<0>(*o3) == 0);
  CHECK(std::get<1>(*o3) == 1);
}

TEST(Optional, basics_int) {
  dmlc::optional<int> x;
  CHECK(!bool(x));
  x = 1;
  CHECK(bool(x));
  CHECK_EQ(x.value(), 1);
  x = dmlc::nullopt;
  CHECK(!bool(x));
  x = 1;
  dmlc::optional<int> y;
  y = x;
  CHECK_EQ(y.value(), 1);
}

TEST(Optional, parsing_int) {
  dmlc::optional<int> x;
  {
    std::ostringstream os;
    os << x;
    CHECK_EQ(os.str(), "None");
  }

  {
    x = 1;
    std::ostringstream os;
    os << x;
    CHECK_EQ(os.str(), "1");
  }

  {
    std::string none("None");
    std::istringstream is(none);
    is >> x;
    CHECK(!bool(x));
  }

  {
    std::string one("1");
    std::istringstream is(one);
    is >> x;
    CHECK_EQ(x.value(), 1);
  }
}

struct OptionalParamInt : public dmlc::Parameter<OptionalParamInt> {
  dmlc::optional<int> none;
  dmlc::optional<int> one;
  dmlc::optional<int> long_one;
  dmlc::optional<int> def;

  DMLC_DECLARE_PARAMETER(OptionalParamInt) {
    DMLC_DECLARE_FIELD(none).add_enum("one", 1);
    DMLC_DECLARE_FIELD(one).add_enum("one", 1);
    DMLC_DECLARE_FIELD(long_one);
    DMLC_DECLARE_FIELD(def).add_enum("one", 1).set_default(dmlc::optional<int>());
  }
};

DMLC_REGISTER_PARAMETER(OptionalParamInt);

TEST(Optional, add_enum_int) {
  OptionalParamInt param;
  std::map<std::string, std::string> kwargs;
  kwargs["none"] = "None";
  kwargs["one"] = "one";
  kwargs["long_one"] = "1L";
  param.Init(kwargs);
  CHECK(!param.none);
  CHECK_EQ(param.one.value(), 1);
  CHECK_EQ(param.long_one.value(), 1);
  CHECK(!param.def);
}

// Repeat above tests, but now testing optional<bool> rather than optional<int>

TEST(Optional, basics_bool) {
  dmlc::optional<bool> x;
  CHECK(!bool(x));
  x = true;
  CHECK(bool(x));
  CHECK_EQ(x.value(), true);
  x = dmlc::nullopt;
  CHECK(!bool(x));
  x = true;
  dmlc::optional<bool> y;
  y = x;
  CHECK_EQ(y.value(), true);
  x = false;
  y = x;
  CHECK_EQ(y.value(), false);
}

TEST(Optional, parsing_bool) {
  dmlc::optional<bool> x;
  dmlc::optional<bool> y;
  {
    std::ostringstream os;
    os << x;
    CHECK_EQ(os.str(), "None");
  }

  {
    x = true;
    std::ostringstream os;
    os << x;
    CHECK_EQ(os.str(), "1");
  }

  {
    x = false;
    std::ostringstream os;
    os << x;
    CHECK_EQ(os.str(), "0");
  }

  {
    std::string none("None");
    std::istringstream is(none);
    is >> x;
    CHECK(!bool(x));
  }

  {
    std::string one("1");
    std::istringstream is(one);
    is >> x;
    CHECK_EQ(x.value(), true);
  }

  {
    std::string zero("0");
    std::istringstream is(zero);
    is >> x;
    CHECK_EQ(x.value(), false);
  }

  {
    std::string one("true");
    std::istringstream is(one);
    is >> x;
    CHECK_EQ(x.value(), true);
  }

  {
    std::string zero("false");
    std::istringstream is(zero);
    is >> x;
    CHECK_EQ(x.value(), false);
  }

  {
    std::istringstream is("false true");
    is >> x >> y;
    CHECK_EQ(x.value(), false);
    CHECK_EQ(y.value(), true);
  }
}

struct OptionalParamBool : public dmlc::Parameter<OptionalParamBool> {
  dmlc::optional<bool> none;
  dmlc::optional<bool> none_with_default;
  dmlc::optional<bool> set_to_none;

  DMLC_DECLARE_PARAMETER(OptionalParamBool) {
    DMLC_DECLARE_FIELD(none);
    DMLC_DECLARE_FIELD(none_with_default).set_default(dmlc::optional<bool>());
    DMLC_DECLARE_FIELD(set_to_none);
  }
};

DMLC_REGISTER_PARAMETER(OptionalParamBool);

TEST(Optional, bool_in_struct) {
  OptionalParamBool param;
  CHECK(!param.none);
  // With optional<bool>, the following explicit approach avoids confusion.
  CHECK(!param.none.has_value());
  CHECK(!param.none_with_default);
  std::map<std::string, std::string> kwargs;
  // Assign new logical values, testing string assignment
  kwargs["none"] = "0";
  kwargs["none_with_default"] = "true";
  kwargs["set_to_none"] = "None";
  param.Init(kwargs);
  CHECK(param.none);
  CHECK(!param.none.value());
  CHECK(param.none_with_default);
  CHECK(param.none_with_default.value());
  CHECK(!param.set_to_none.has_value());
}
