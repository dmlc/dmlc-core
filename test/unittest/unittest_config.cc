#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <dmlc/config.h>
#include <gtest/gtest.h>

using namespace std;

TEST(Config, basics) {
  string cfg_str =
    "k1=1243\n"
    "k2=0.5\n"
    "k3=\"abc\"\n"
    "k4=\"wmj\"\n"
    "k5=\"x=1\"\n"
    "k6=\"hello world\"\n"
    "k7=\"quote\\\"quote\"\n"
    "#i am comment\n"
    "#i am evil comment x=1\n"
    "k8=-1.2  #comment \n"
    "k9=10\n"
    "k10=\"#not comment\"\n"
    ;
  istringstream iss(cfg_str);
  using namespace dmlc;
  SimpleConfig cfg(iss);
  for(const auto& entry : cfg) {
    cout << "k: " << entry.first << "\tv: " << entry.second << endl;
  }
  cout << "Proto string:" << endl;
  cout << cfg.ToProtoString() << endl;
}

TEST(Config, duplicate_keys) {
  string cfg_str = 
    "k = 0.1\n"
    "k = 0.3\n"
    "k = 0.5\n"
    ;
  {
    cout << ">>>> multi map " << endl;
    istringstream iss(cfg_str);
    using namespace dmlc;
    MultiConfig cfg(iss);
    for(const auto& entry : cfg) {
      cout << "k: " << entry.first << "\tv: " << entry.second << endl;
    }
    cout << "Proto string:" << endl;
    cout << cfg.ToProtoString() << endl;
  }
  {
    cout << ">>>> simple map " << endl;
    istringstream iss(cfg_str);
    using namespace dmlc;
    SimpleConfig cfg(iss);
    for(const auto& entry : cfg) {
      cout << "k: " << entry.first << "\tv: " << entry.second << endl;
    }
    cout << "Proto string:" << endl;
    cout << cfg.ToProtoString() << endl;
  }
}

TEST(Config, set_params) {
  using namespace dmlc;
  SimpleConfig cfg;
  cfg.SetParam("k1", 1);
  cfg.SetParam("k2", "123", true);
  cout << cfg.ToProtoString() << endl;
}
