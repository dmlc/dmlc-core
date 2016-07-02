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
  dict2["1"].get<int>() += 1;

  CHECK_EQ(dict2["1"].get<int>(), 2);
  CHECK_EQ(dict2["vec"].get<std::vector<int> >()[1], 2);
}

TEST(Any, cover) {
  dmlc::any a = std::string("abc");
  dmlc::any b = 1;

  CHECK_EQ(a.get<std::string>(), "abc");
  a = std::move(b);
  CHECK(b.empty());
  CHECK_EQ(a.get<int>(), 1);

  std::shared_ptr<int> x = std::make_shared<int>(10);
  {
    dmlc::any aa(x);
    CHECK_EQ(*aa.get<std::shared_ptr<int> >(), 10);
  }
  // aa must be destructed.
  CHECK(x.unique());
}
