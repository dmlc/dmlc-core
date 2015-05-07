#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <dmlc/config.h>
#include <gtest/gtest.h>

using namespace std;

TEST(Config, basics) {
  ifstream fin("test.cfg");
  using namespace dmlc;
  Config cfg(fin);
  for(const auto& entry : cfg) {
    cout << "k: " << entry.first << "\tv: " << entry.second << endl;
  }
  cout << "Proto string:" << endl;
  cout << cfg.ToProtoString() << endl;
}
