#include <dmlc/logging.h>
#include <gtest/gtest.h>

using namespace std;

TEST(Logging, basics) {
  LOG(INFO) << "hello";
  LOG(ERROR) << "error";

  int x = 1, y = 1;
  CHECK_EQ(x, y);
  CHECK_GE(x, y);
  ASSERT_DEATH(CHECK_NE(x, y), ".*");
}
