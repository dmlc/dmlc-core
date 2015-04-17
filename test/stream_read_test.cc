// test reading speed from a Stream
#include <cstdlib>
#include <cstdio>
#include <dmlc/io.h>
#include <dmlc/timer.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: uri buffersize\n");
    return 0;
  }
  size_t sz = atol(argv[2]);
  std::string buffer; buffer.resize(sz);
  using namespace dmlc;
  Stream *fi = Stream::Create(argv[1], "r");
  double tstart = GetTime();
  size_t size;
  size_t bytes_read = 0;
  size_t bytes_expect = 10UL << 20UL;
  while ((size = fi->Read(BeginPtr(buffer), sz)) != 0) {
    bytes_read += size;
    double tdiff = GetTime() - tstart;
    if (bytes_read >= bytes_expect) {
      printf("%lu MB read, %g MB/sec\n",
             bytes_read >> 20UL,
             (bytes_read >> 20UL) / tdiff);
      bytes_expect += 10UL << 20UL;
    }
  }
  delete fi;
  return 0;
}

