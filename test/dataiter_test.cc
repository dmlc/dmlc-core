#include <dmlc/data.h>

int main(void) {
  using namespace dmlc;
  IDataIter<RowBlock<index_t> > *iter
      = RowBlock<index_t>::CreateIter(NULL, "");
  while (iter->Next()) {
    const RowBlock<index_t> &batch = iter->Value();
    for (size_t i = 0; i < batch.size; ++i) {
      Row<index_t> row = batch[i];      
    }
  }
  delete iter;
  return 0;
}
