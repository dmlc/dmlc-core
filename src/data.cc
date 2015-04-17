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
CreateIter_(InputSplit *source,
            const std::string &cfg) {
  // todo new parser
  return NULL;
  //return new BasicRowIter<IndexType>(new LibSVMParser(source));
}
}  // namespace data
template<>
RowBlockIter<unsigned> *
RowBlockIter<unsigned>::Create(InputSplit *source,
                               const std::string &cfg) {
  return data::CreateIter_<unsigned>(source, cfg);
}
}  // dmlc
