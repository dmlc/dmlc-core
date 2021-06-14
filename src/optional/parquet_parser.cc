#include <dmlc/data.h>
#include <dmlc/registry.h>
#include "data/parquet_parser.h"

namespace dmlc {
namespace data {

template<typename IndexType, typename DType = real_t>
Parser<IndexType> *
CreateParquetParser(const std::string& path,
                    const std::map<std::string, std::string>& args,
                    unsigned part_index,
                    unsigned num_parts) {
  ParserImpl<IndexType> *parser = new ParquetParser<IndexType>(path, args, 2);
  return parser;
}

DMLC_REGISTER_PARAMETER(ParquetParserParam);
}  // namespace data

DMLC_REGISTER_DATA_PARSER(
  uint32_t, real_t, parquet, data::CreateParquetParser<uint32_t __DMLC_COMMA real_t>);
DMLC_REGISTER_DATA_PARSER(
  uint64_t, real_t, parquet, data::CreateParquetParser<uint64_t __DMLC_COMMA real_t>);
}  // namespace dmlc
