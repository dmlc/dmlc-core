#include <gtest/gtest.h>
#include <dmlc/parameter.h>

struct LearningParam : public dmlc::Parameter<LearningParam> {
  float float_param;
  double double_param;
  DMLC_DECLARE_PARAMETER(LearningParam) {
      DMLC_DECLARE_FIELD(float_param).set_default(0.01f);
      DMLC_DECLARE_FIELD(double_param).set_default(0.1);
  }
};

DMLC_REGISTER_PARAMETER(LearningParam);

TEST(Parameter, parsing_float) {
  LearningParam param;
  std::map<std::string, std::string> kwargs;

  kwargs["float_param"] = "0";
  ASSERT_NO_THROW(param.Init(kwargs));
  kwargs["float_param"] = "0.015625";  // can be represented exactly in IEEE 754
  ASSERT_NO_THROW(param.Init(kwargs));
  ASSERT_EQ(param.float_param, 0.015625f);
  kwargs["float_param"] = "-0.015625";  // can be represented exactly in IEEE 754
  ASSERT_NO_THROW(param.Init(kwargs));
  ASSERT_EQ(param.float_param, -0.015625f);

  kwargs["float_param"] = "1e-10";
  ASSERT_NO_THROW(param.Init(kwargs));
  kwargs["float_param"] = "1e10";
  ASSERT_NO_THROW(param.Init(kwargs));

  kwargs["float_param"] = "1e-100";
  ASSERT_THROW(param.Init(kwargs), dmlc::ParamError);
  kwargs["float_param"] = "1e100";
  ASSERT_THROW(param.Init(kwargs), dmlc::ParamError);

  kwargs = std::map<std::string, std::string>();

  kwargs["double_param"] = "0";
  ASSERT_NO_THROW(param.Init(kwargs));
  kwargs["double_param"] = "0.00048828125";  // can be represented exactly in IEEE 754
  ASSERT_NO_THROW(param.Init(kwargs));
  ASSERT_EQ(param.double_param, 0.00048828125);
  kwargs["double_param"] = "-0.00048828125";  // can be represented exactly in IEEE 754
  ASSERT_NO_THROW(param.Init(kwargs));
  ASSERT_EQ(param.double_param, -0.00048828125);

  kwargs["double_param"] = "1e-10";
  ASSERT_NO_THROW(param.Init(kwargs));
  kwargs["double_param"] = "1e10";
  ASSERT_NO_THROW(param.Init(kwargs));
  kwargs["double_param"] = "1e-100";
  ASSERT_NO_THROW(param.Init(kwargs));
  kwargs["double_param"] = "1e100";
  ASSERT_NO_THROW(param.Init(kwargs));

  kwargs["double_param"] = "1e-500";
  ASSERT_THROW(param.Init(kwargs), dmlc::ParamError);
  kwargs["double_param"] = "1e500";
  ASSERT_THROW(param.Init(kwargs), dmlc::ParamError);
}
