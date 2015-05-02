#include "../src/data/strtonum.h"
#include <dmlc/logging.h>

int main(int argc, char *argv[]) {
  using namespace dmlc;

  std::cout << data::atof("-0123.452e-2") << std::endl;
  std::cout << data::atol("-23498") << std::endl;
  return 0;
}
