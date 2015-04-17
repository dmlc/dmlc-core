// test reading speed from a InputSplit
#include <cstdlib>
#include <cstdio>
#include <dmlc/io.h>
#include <dmlc/timer.h>

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
  double tstart = GetTime();
  size_t bytes_read = 0;
  size_t bytes_expect = 10UL << 20UL;
  while ((size = split->Read(BeginPtr(buffer), sz)) != 0) {
    bytes_read += size;
    double tdiff = GetTime() - tstart;
    if (bytes_read >= bytes_expect) {
      printf("%lu MB read, %g MB/sec\n",
             bytes_read >> 20UL,
             (bytes_read >> 20UL) / tdiff);
      bytes_expect += 10UL << 20UL;
    }
  }
  delete split;
  return 0;
}

