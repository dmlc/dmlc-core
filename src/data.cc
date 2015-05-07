// use direct path for to save compile flags
#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <string>
#include <dmlc/base.h>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include "io/uri_spec.h"
#include "data/parser.h"
#include "data/basic_row_iter.h"
#include "data/disk_row_iter.h"
#include "data/libsvm_parser.h"
#include "data/criteo_parser.h"

namespace dmlc {
/*! \brief namespace for useful input data structure */
namespace data {
template<typename IndexType>
static RowBlockIter<IndexType> *
CreateIter_(const char *uri_,
            unsigned part_index,
            unsigned num_parts,
            const char *type) {
  using namespace std;
  io::URISpec spec(uri_, part_index, num_parts);
  // create parser
  Parser<IndexType> *parser = NULL;
  InputSplit* source = InputSplit::Create(
      spec.uri.c_str(), part_index, num_parts, "text");
  if (!strcmp(type, "libsvm")) {
parser = new LibSVMParser<IndexType>(source, 2);
  } if (!strcmp(type, "criteo")) {
    parser = new CriteoParser<IndexType>(source);
  } else {
    LOG(FATAL) << "unknown datatype " << type;
  }
  if (spec.cache_file.length() != 0) {
#if DMLC_USE_CXX11
	parser = new ThreadedParser<IndexType>(parser);
    return new DiskRowIter<IndexType>(parser, spec.cache_file.c_str(), true);
#else
    LOG(FATAL) << "compile with c++0x or c++11 to enable cache file";
    return NULL;
#endif
  } else {
    return new BasicRowIter<IndexType>(parser);
  }
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
