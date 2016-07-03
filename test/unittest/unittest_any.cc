// Copyright by Contributors

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <dmlc/any.h>
#include <gtest/gtest.h>


TEST(Any, basics) {
  std::unordered_map<std::string, dmlc::any> dict;
  dict["1"] = 1;
  dict["vec"] = std::vector<int>{1,2,3};
  dict["shapex"] = std::string("xtyz");
  std::unordered_map<std::string, dmlc::any> dict2(std::move(dict));
  dmlc::get<int>(dict2["1"]) += 1;

  CHECK_EQ(dmlc::get<int>(dict2["1"]), 2);
  CHECK_EQ(dmlc::get<std::vector<int> >(dict2["vec"])[1], 2);
}

TEST(Any, cover) {
  dmlc::any a = std::string("abc");
  dmlc::any b = 1;

  CHECK_EQ(dmlc::get<std::string>(a), "abc");
  a = std::move(b);
  CHECK(b.empty());
  CHECK_EQ(dmlc::get<int>(a), 1);

  std::shared_ptr<int> x = std::make_shared<int>(10);
  {
    dmlc::any aa(x);
    CHECK_EQ(*dmlc::get<std::shared_ptr<int> >(aa), 10);
  }
  // aa must be destructed.
  CHECK(x.unique());
}
