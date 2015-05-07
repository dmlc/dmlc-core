/**
 * @file   criteo_parser.h
 * @brief  parse criteo ctr data format
 */
#ifndef DMLC_DATA_CRITEO_PARSER_H_
#define DMLC_DATA_CRITEO_PARSER_H_
#include <limits>
#include "./row_block.h"
#include "./parser.h"
#include "./strtonum.h"
#include "./crc32.h"
namespace dmlc {
namespace data {

/**
 * \brief criteo ctr dataset:
 * The columns are tab separeted with the following schema:
 *  <label> <integer feature 1> ... <integer feature 13>
 *  <categorical feature 1> ... <categorical feature 26>
 */
template <typename IndexType>
class CriteoParser : public Parser<IndexType> {
 public:
  explicit CriteoParser(InputSplit *source)
      : bytes_read_(0), source_(source) { }
  virtual ~CriteoParser() {
    delete source_;
  }

  virtual void BeforeFirst(void) {
    source_->BeforeFirst();
  }
  virtual size_t BytesRead(void) const {
    return bytes_read_;
  }
  virtual bool ParseNext(std::vector<RowBlockContainer<IndexType> > *data) {
    InputSplit::Blob chunk;
    if (!source_->NextChunk(&chunk)) return false;
    CHECK(chunk.size != 0);
    bytes_read_ += chunk.size;
    char *p = reinterpret_cast<char*>(chunk.dptr);
    char *end = p + chunk.size;
    data->resize(1);
    RowBlockContainer<IndexType>& blk = (*data)[0];
    IndexType kmax = std::numeric_limits<IndexType>::max();
    IndexType itv = kmax / 13;
    char *pp = p;
    while (p != end) {
      while (isspace(*p) && p != end) ++p;
      if (p == end) break;

      // parse label
      blk.label.push_back(atof(p));
      p = pp + 1;

      // parse inter feature
      for (IndexType os = 0; os < kmax; os += itv) {
        pp = strchr(p, '\t');
        CHECK_NOTNULL(pp);
        if (pp > p) {
          blk.index.push_back(os + atol(p));
        }
        p = pp + 1;
      }

      // parse categorty feature
      for (int i = 0; i < 26; ++i) {
        pp = strchr(p, '\t');
        size_t len = pp - p;
        if (pp == NULL) { CHECK_EQ(i, 25); len = strlen(p); }
        if (!pp) continue;
        blk.index.push_back(CRC32HW(p, len));
      }
      blk.offset.push_back(blk.index.size());
    }
    return true;
  }

 private:
  // number of bytes readed
  size_t bytes_read_;
  // source split that provides the data
  InputSplit *source_;
};

}  // namespace data
}  // namespace dmlc


#endif /* DMLC_DATA_CRITEO_PARSER_H_ */
