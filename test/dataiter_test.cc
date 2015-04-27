#include <dmlc/data.h>
#include <dmlc/timer.h>

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Usage: <libsvm> partid npart\n");
    return 0;
  }
  using namespace dmlc;
  RowBlockIter<index_t> *iter = 
      RowBlockIter<index_t>::Create(argv[1],
                                    atoi(argv[2]),
                                    atoi(argv[3]),
                                    "libsvm");
  double tstart = GetTime();
  size_t bytes_read = 0;
  while (iter->Next()) {
    const RowBlock<index_t> &batch = iter->Value();
    bytes_read += batch.MemCostBytes();
    double tdiff = GetTime() - tstart;
    LOG(INFO) << (bytes_read >> 20UL) <<
        " MB read " << ((bytes_read >> 20UL) / tdiff)<< " MB/sec";
  }
  return 0;
}
