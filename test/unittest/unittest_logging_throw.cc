// Copyright by Contributors
#define DMLC_LOG_FATAL_THROW 1

#include <dmlc/logging.h>
#include <gtest/gtest.h>

TEST(LoggingThrow, exception) {
  EXPECT_THROW({
    LOG(FATAL) << "message";
  }, dmlc::Error);
}
