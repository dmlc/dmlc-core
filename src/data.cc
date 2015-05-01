// use direct path for to save compile flags
#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <string>
#include <dmlc/base.h>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include "data/parser.h"
#include "data/basic_row_iter.h"
#include "data/disk_row_iter.h"
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
  // allow cachefile in format path#cachefile
  std::string fname_;
  const char *cache_file = NULL;
  const char *dlm = strchr(uri, '#');
  if (dlm != NULL) {
    CHECK(strchr(dlm + 1, '#') == NULL)
        << "only one `#` is allowed in file path for cachefile specification";
    fname_ = std::string(uri, dlm - uri);
    uri = fname_.c_str();
    cache_file = dlm + 1;
  }
  // create parser
  Parser *parser = NULL;
  if (!strcmp(type, "libsvm")) {
    parser = new LibSVMParser(InputSplit::Create(uri, part_index, num_parts,
                                                 "text"), 16);
  } else {
    LOG(FATAL) << "unknown datatype " << type;
  }
  if (cache_file != NULL) {
#if DMLC_USE_CXX11
	parser = new ThreadedParser(parser);
    return new DiskRowIter<IndexType>(parser, cache_file, true);
#else
    LOG(FATAL) << "compile with c++0x or c++11 to enable cache file";
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
