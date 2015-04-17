/*!
 *  Copyright (c) 2015 by Contributors
 * \file libsvm_parser.h
 * \brief iterator parser to parse libsvm format
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_LIBSVM_PARSER_H_
#define DMLC_DATA_LIBSVM_PARSER_H_

#include <vector>
#include <sstream>
#include <dmlc/data.h>

namespace dmlc {
namespace data {
/*!
 * \brief libsvm parser that parses the input lines
 *  and returns rows 
 */
class LibSVMParser : public DataIter<Row<size_t> > {
 public:
  explicit LibSVMParser(InputSplit *source)
      : is_(source, kBufferSize),
        source_(source), at_head_(true) {
    if (!(is_ >> tmp_)) {
      tmp_.resize(0);
    }
  }
  virtual ~LibSVMParser() {
    is_.set_stream(NULL);
    delete source_;
  }
  virtual void BeforeFirst(void) {
    CHECK(at_head_) << "cannot call BeforeFirst on LibSVMIter";
  }
  virtual const Row<size_t> &Value(void) const {
    return row_;
  }
  virtual bool Next(void) {
    if (tmp_.length() == 0) return false;
    size_t index;
    float value;
    CHECK(sscanf(tmp_.c_str(), "%f", &value) == 1)
        << "bad libsvm format";
    row_.label = static_cast<real_t>(value);
    findex_.clear(); fvalue_.clear();
    while (is_ >> tmp_) {
      if (sscanf(tmp_.c_str(), "%lu:%f", &index, &value) == 2) {
        findex_.push_back(index);
        fvalue_.push_back(value);
      } else {
        break;
      }
    }
    row_.index = BeginPtr(findex_);
    row_.value = BeginPtr(fvalue_);
    return true;
  }
  inline size_t bytes_read(void) const {
    return is_.bytes_read();
  }

 private:
  // buffer size
  static const int kBufferSize = 2 << 10;
  // internal temp string
  std::string tmp_;
  // input stream
  dmlc::istream is_;
  // source split that provides the data
  InputSplit *source_;
  // temporal string
  std::string temp_;
  // at beginning
  bool at_head_;
  // the returning row
  Row<size_t> row_;
  // feature index
  std::vector<size_t> findex_;
  // feature value
  std::vector<real_t> fvalue_;
};
}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_LIBSVM_ITER_H_
