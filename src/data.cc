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
  ParserImpl<IndexType> *parser = NULL;
  InputSplit* source = InputSplit::Create(
      spec.uri.c_str(), part_index, num_parts, "text");
  if (!strcmp(type, "libsvm")) {
    parser = new LibSVMParser<IndexType>(source, 2);
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

template<typename IndexType>
static Parser<IndexType> *
CreateParser_(const char *uri_,
unsigned part_index,
unsigned num_parts,
const char *type) {
  using namespace std;
  io::URISpec spec(uri_, part_index, num_parts);
  // create parser
  ParserImpl<IndexType> *parser = NULL;
  InputSplit* source = InputSplit::Create(
    spec.uri.c_str(), part_index, num_parts, "text");
  if (!strcmp(type, "libsvm")) {
    parser = new LibSVMParser<IndexType>(source, 2);
  }
  else {
    LOG(FATAL) << "unknown datatype " << type;
  }
  if (spec.cache_file.length() != 0) {
#if DMLC_USE_CXX11
    parser = new ThreadedParser<IndexType>(parser);
    return parser;
#else
    LOG(FATAL) << "compile with c++0x or c++11 to enable cache file";
    return NULL;
#endif
  }
  else {
    return parser;
  }
}

}  // namespace data

template<>
RowBlockIter<uint32_t> *
RowBlockIter<uint32_t>::Create(const char *uri,
                               unsigned part_index,
                               unsigned num_parts,
                               const char *type) {
  return data::CreateIter_<uint32_t>(uri, part_index, num_parts, type);
}

template<>
RowBlockIter<int32_t> *
RowBlockIter<int32_t>::Create(const char *uri,
unsigned part_index,
unsigned num_parts,
const char *type) {
  return data::CreateIter_<int32_t>(uri, part_index, num_parts, type);
}

template<>
RowBlockIter<uint64_t> *
RowBlockIter<uint64_t>::Create(const char *uri,
unsigned part_index,
unsigned num_parts,
const char *type) {
  return data::CreateIter_<uint64_t>(uri, part_index, num_parts, type);
}

template<>
RowBlockIter<int64_t> *
RowBlockIter<int64_t>::Create(const char *uri,
unsigned part_index,
unsigned num_parts,
const char *type) {
  return data::CreateIter_<int64_t>(uri, part_index, num_parts, type);
}

template<>
Parser<int32_t> * 
Parser<int32_t>::Create(const char *uri_,
  unsigned part_index,
  unsigned num_parts,
  const char *type) {
  return data::CreateParser_<int32_t>(uri_, part_index, num_parts, type);
}

template<>
Parser<uint32_t> *
Parser<uint32_t>::Create(const char *uri_,
unsigned part_index,
unsigned num_parts,
const char *type) {
  return data::CreateParser_<uint32_t>(uri_, part_index, num_parts, type);
}

template<>
Parser<int64_t> *
Parser<int64_t>::Create(const char *uri_,
unsigned part_index,
unsigned num_parts,
const char *type) {
  return data::CreateParser_<int64_t>(uri_, part_index, num_parts, type);
}

template<>
Parser<uint64_t> *
Parser<uint64_t>::Create(const char *uri_,
unsigned part_index,
unsigned num_parts,
const char *type) {
  return data::CreateParser_<uint64_t>(uri_, part_index, num_parts, type);
}
}  // dmlc
