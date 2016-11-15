// Copyright by Contributors
#include <dmlc/base.h>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include <dmlc/data.h>
#include <dmlc/registry.h>
#include <cstring>
#include <string>
#include "io/uri_spec.h"
#include "data/parser.h"
#include "data/basic_row_iter.h"
#include "data/disk_row_iter.h"
#include "data/libsvm_parser.h"
#include "data/csv_parser.h"

namespace dmlc {
/*! \brief namespace for useful input data structure */
namespace data {

template<typename IndexType>
Parser<IndexType> *
CreateLibSVMParser(const std::string& path,
                   const std::map<std::string, std::string>& args,
                   unsigned part_index,
                   unsigned num_parts) {
  InputSplit* source = InputSplit::Create(
      path.c_str(), part_index, num_parts, "text");
  ParserImpl<IndexType> *parser = new LibSVMParser<IndexType>(source, 2);
#if DMLC_ENABLE_STD_THREAD
  parser = new ThreadedParser<IndexType>(parser);
#endif
  return parser;
}

template<typename IndexType>
Parser<IndexType> *
CreateCSVParser(const std::string& path,
                const std::map<std::string, std::string>& args,
                unsigned part_index,
                unsigned num_parts) {
  InputSplit* source = InputSplit::Create(
      path.c_str(), part_index, num_parts, "text");
  return new CSVParser<IndexType>(source, args, 2);
}

template<typename IndexType>
inline Parser<IndexType> *
CreateParser_(const char *uri_,
              unsigned part_index,
              unsigned num_parts,
              const char *type,
              const std::map<std::string, std::string>& args) {
  std::string ptype = type;
  io::URISpec spec(uri_, part_index, num_parts);
  if (ptype == "auto") {
    if (spec.args.count("format") != 0) {
      ptype = spec.args.at("format");
    } else {
      ptype = "libsvm";
    }
  }

  const ParserFactoryReg<IndexType>* e =
      Registry<ParserFactoryReg<IndexType> >::Get()->Find(ptype);
  if (e == NULL) {
    LOG(FATAL) << "Unknown data type " << ptype;
  }
  std::map<std::string, std::string> parser_args;
  // `spec.args` is used only if the key is not present in `args`
  parser_args.insert(args.begin(), args.end());
  parser_args.insert(spec.args.begin(), spec.args.end());
  // create parser
  return (*e->body)(spec.uri, parser_args, part_index, num_parts);
}

template<typename IndexType>
inline RowBlockIter<IndexType> *
CreateIter_(const char *uri_,
            unsigned part_index,
            unsigned num_parts,
            const char *type,
            const std::map<std::string, std::string>& args) {
  using namespace std;
  io::URISpec spec(uri_, part_index, num_parts);
  Parser<IndexType> *parser = CreateParser_<IndexType>
      (spec.uri.c_str(), part_index, num_parts, type, args);
  if (spec.cache_file.length() != 0) {
#if DMLC_ENABLE_STD_THREAD
    return new DiskRowIter<IndexType>(parser, spec.cache_file.c_str(), true);
#else
    LOG(FATAL) << "compile with c++0x or c++11 to enable cache file";
    return NULL;
#endif
  } else {
    return new BasicRowIter<IndexType>(parser);
  }
}

DMLC_REGISTER_PARAMETER(CSVParserParam);
}  // namespace data

// template specialization
template<>
RowBlockIter<uint32_t> *
RowBlockIter<uint32_t>::Create(const char *uri,
                               unsigned part_index,
                               unsigned num_parts,
                               const char *type,
                               const std::map<std::string, std::string>& args) {
  return data::CreateIter_<uint32_t>(uri, part_index, num_parts, type, args);
}

template<>
RowBlockIter<uint64_t> *
RowBlockIter<uint64_t>::Create(const char *uri,
                               unsigned part_index,
                               unsigned num_parts,
                               const char *type,
                               const std::map<std::string, std::string>& args) {
  return data::CreateIter_<uint64_t>(uri, part_index, num_parts, type, args);
}

template<>
Parser<uint32_t> *
Parser<uint32_t>::Create(const char *uri_,
                         unsigned part_index,
                         unsigned num_parts,
                         const char *type,
                         const std::map<std::string, std::string>& args) {
  return data::CreateParser_<uint32_t>(uri_, part_index, num_parts, type, args);
}

template<>
Parser<uint64_t> *
Parser<uint64_t>::Create(const char *uri_,
                         unsigned part_index,
                         unsigned num_parts,
                         const char *type,
                         const std::map<std::string, std::string>& args) {
  return data::CreateParser_<uint64_t>(uri_, part_index, num_parts, type, args);
}

// registry
DMLC_REGISTRY_ENABLE(ParserFactoryReg<uint32_t>);
DMLC_REGISTRY_ENABLE(ParserFactoryReg<uint64_t>);
DMLC_REGISTER_DATA_PARSER(uint32_t, libsvm, data::CreateLibSVMParser<uint32_t>);
DMLC_REGISTER_DATA_PARSER(uint64_t, libsvm, data::CreateLibSVMParser<uint64_t>);

DMLC_REGISTER_DATA_PARSER(uint32_t, csv, data::CreateCSVParser<uint32_t>);

}  // namespace dmlc
