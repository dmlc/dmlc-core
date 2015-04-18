// use direct path for to save compile flags
#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include "data/basic_row_iter.h"
#include "data/libsvm_parser.h"

namespace dmlc {
/*! \brief namespace for useful input data structure */
namespace data {
template<typename IndexType>
static RowBlockIter<IndexType> *
CreateIter_(const char *uri,
            unsigned part_index,
            unsigned num_parts,
            const char *type) {
  using namespace std;
  if (!strcmp(type, "libsvm")) {
    return new BasicRowIter<IndexType>(
        new LibSVMParser(InputSplit::Create(uri, part_index, num_parts,
                                            "text"), 4));
  }
  LOG(FATAL) << "unknown dataset type " << type;
  return NULL;
}
}  // namespace data
template<>
RowBlockIter<unsigned> *
RowBlockIter<unsigned>::Create(const char *uri,
                               unsigned part_index,
                               unsigned num_parts,
                               const char *type) {
  return data::CreateIter_<unsigned>(uri, part_index, num_parts, type);
}
}  // dmlc
