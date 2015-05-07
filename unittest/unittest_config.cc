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
    "k3=abc\n"
    "k4=\"wmj\"\n"
    "k5=x=1\n"
    "k6=\n"
    "=\n"
    "   \n"
    "xxx\n"
    "k7=\"hello world\"\n"
    "k8=\\t\n";
  istringstream iss(cfg_str);
  using namespace dmlc;
  Config cfg(iss);
  for(const auto& entry : cfg) {
    cout << "k: " << entry.first << "\tv: " << entry.second << endl;
  }
  cout << "Proto string:" << endl;
  cout << cfg.ToProtoString() << endl;
}
