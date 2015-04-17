// test reading speed from a InputSplit
#include <cstdlib>
#include <cstdio>
#include <dmlc/io.h>
#include <dmlc/timer.h>
#include "../src/data/libsvm_parser.h"

int main(int argc, char *argv[]) {
  if (argc < 6) {
    printf("Usage: <libsvm> partid npart buffer\n");
    return 0;
  }
  using namespace dmlc;
  InputSplit *split = InputSplit::Create(argv[1],
                                         atoi(argv[2]),
                                         atoi(argv[3]),
                                         "text");
  size_t sz = atol(argv[4]);
  int nthread = atoi(argv[5]);
  data::LibSVMParser parser(split, sz, nthread);
  double tstart = GetTime();
  size_t bytes_read = 0;
  size_t bytes_expect = 10UL << 20UL;
  while (parser.Next()) {
    bytes_read  = parser.bytes_read();
    double tdiff = GetTime() - tstart;
    if (bytes_read >= bytes_expect) {
      printf("%lu MB read, %g MB/sec\n",
             bytes_read >> 20UL,
             (bytes_read >> 20UL) / tdiff);
      bytes_expect += 10UL << 20UL;
    }
  }
  return 0;
}
