#include <gtest/gtest.h>
#include <dmlc/parameter.h>

struct LearningRateParam : public dmlc::Parameter<LearningRateParam> {
  float learning_rate;
  DMLC_DECLARE_PARAMETER(LearningRateParam) {
      DMLC_DECLARE_FIELD(learning_rate).set_default(0.01);
  }
};

DMLC_REGISTER_PARAMETER(LearningRateParam);

TEST(Parameter, parsing_small_float) {
  LearningRateParam param;
  std::map<std::string, std::string> kwargs;
  kwargs["learning_rate"] = "1e-100";
  EXPECT_THROW(
      param.Init(kwargs),
      dmlc::ParamError
  );
}
