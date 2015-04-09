#include <dmlc/data.h>

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Usage: <libsvm> partid npart\n");
    return 0;
  }
  using namespace dmlc;
  DataIter<RowBlock<index_t> > *iter
      = RowBlock<index_t>::CreateIter
      (InputSplit::Create(argv[1],
                          atoi(argv[2]),
                          atoi(argv[3])));
  while (iter->Next()) {
    const RowBlock<index_t> &batch = iter->Value();
    for (size_t i = 0; i < batch.size; ++i) {
      Row<index_t> row = batch[i];
      printf("%g", row.label);
      for (size_t i = 0; i < row.length; ++i) {
        printf(" %u:%g", row.index[i], row.value[i]);
      }
      printf("\n");
    }
  }
  delete iter;
  return 0;
}
