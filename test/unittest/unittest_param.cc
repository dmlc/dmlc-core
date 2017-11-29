/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

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
  kwargs["learning_rate"] = "9.4039548065783e-39";
  EXPECT_THROW(
      param.Init(kwargs),
      dmlc::ParamError
  );
}
