// Copyright by Contributors
#define DMLC_LOG_FATAL_THROW 1

#include <dmlc/logging.h>
#include <gtest/gtest.h>

TEST(Logging, basics) {
  LOG(INFO) << "hello";
  LOG(ERROR) << "error";

  int x = 1, y = 1;
  CHECK_EQ(x, y);
  CHECK_GE(x, y);

  int *z = &x;
  CHECK_EQ(*CHECK_NOTNULL(z), x);

  EXPECT_THROW(CHECK_NE(x, y), dmlc::Error);
}

TEST(Logging, throw_fatal) {
  EXPECT_THROW({
    LOG(FATAL) << "message";
  }, dmlc::Error);
}
