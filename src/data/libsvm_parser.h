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
      : source_(source), at_head_(true) {
  }
  virtual ~LibSVMParser() {
    delete source_;
  }
  virtual void BeforeFirst(void) {
    CHECK(at_head_) << "cannot call BeforeFirst on LibSVMIter";
  }
  virtual const Row<size_t> &Value(void) const {
    return row_;
  }
  virtual bool Next(void) {
    if (source_->ReadRecord(&temp_)) {
      std::istringstream ss(temp_);
      findex_.clear();
      fvalue_.clear();
      CHECK(ss >> row_.label) << "invalid LIBSVM format";
      size_t findex;
      real_t fvalue;
      while (!ss.eof()) {
        if (!(ss >> findex)) break;
        ss.ignore(32, ':');
        if (!(ss >> fvalue)) break;
        findex_.push_back(findex);
        fvalue_.push_back(fvalue);
      }
      row_.index = BeginPtr(findex_);
      row_.value = BeginPtr(fvalue_);
      row_.length = findex_.size();
      at_head_ = false;
      return true;
    } else {
      return false;
    }
  }
  
 private:
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
