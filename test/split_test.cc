// test reading speed from a InputSplit
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <dmlc/io.h>
#include <dmlc/base.h>

int main(int argc, char *argv[]) {
  if (argc < 5) {
    printf("Usage: <libsvm> partid npart buffer\n");
    return 0;
  }
  using namespace dmlc;
  InputSplit *split = InputSplit::Create(argv[1],
                                         atoi(argv[2]),
                                         atoi(argv[3]),
                                         "text");
  size_t sz = atol(argv[4]);
  size_t size;
  std::string buffer; buffer.resize(sz);
  while ((size = split->Read(BeginPtr(buffer), sz)) != 0) {
    std::cout << std::string(BeginPtr(buffer), size);
  }
  delete split;
  return 0;
}

