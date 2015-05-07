#include <dmlc/logging.h>

int main(int argc, char *argv[]) {

  using namespace dmlc;
  LOG(INFO) << "hello";
  LOG(ERROR) << "error";

  int x = 1, y = 1;
  CHECK_EQ(x, y);
  CHECK_GE(x, y);
  CHECK_NE(x, y);
  return 0;
}
