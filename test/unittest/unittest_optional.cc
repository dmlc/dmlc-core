// Copyright by Contributors

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <dmlc/optional.h>
#include <dmlc/parameter.h>
#include <gtest/gtest.h>


TEST(Optional, basics) {
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

TEST(Optional, parsing) {
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

struct OptionalParam : public dmlc::Parameter<OptionalParam> {
  dmlc::optional<int> none;
  dmlc::optional<int> one;
  dmlc::optional<int> def;

  DMLC_DECLARE_PARAMETER(OptionalParam) {
    DMLC_DECLARE_FIELD(none)
    .add_enum("one", 1);
    DMLC_DECLARE_FIELD(one)
    .add_enum("one", 1);
    DMLC_DECLARE_FIELD(def)
    .add_enum("one", 1)
    .set_default(dmlc::optional<int>());
  }
};

DMLC_REGISTER_PARAMETER(OptionalParam);

TEST(Optional, add_enum) {
  OptionalParam param;
  std::map<std::string, std::string> kwargs;
  kwargs["none"] = "None";
  kwargs["one"] = "one";
  param.Init(kwargs);
  CHECK(!param.none);
  CHECK_EQ(param.one.value(), 1);
  CHECK(!param.def);
}
