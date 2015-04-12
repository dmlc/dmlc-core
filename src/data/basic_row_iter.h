/*!
 *  Copyright (c) 2015 by Contributors
 * \file basic_row_iter.h
 * \brief row based iterator that
 *   loads in everything into memory and returns 
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_BASIC_ROW_ITER_H_
#define DMLC_DATA_BASIC_ROW_ITER_H_
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include <dmlc/data.h>
#include "./row_block.h"

namespace dmlc {
namespace data {
/*!
 * \brief basic set of row iterators that provides
 * \tparam IndexType the type of index we are using
 */
template<typename IndexType>
class BasicRowIter: public RowBlockIter<IndexType> {
 public:
  explicit BasicRowIter(DataIter<Row<size_t> > *parser)
      : at_head_(true) {
    this->Init(parser);
    delete parser;
  }
  virtual ~BasicRowIter() {}
  virtual void BeforeFirst(void) {
    at_head_ = true;
  }
  virtual bool Next(void) {
    if (at_head_) {
      at_head_ = false;
      return true;
    } else {
      return false;
    }
  }
  virtual const RowBlock<IndexType> &Value(void) const {
    return row_;
  }
  virtual size_t NumCol(void) const {
    return static_cast<size_t>(data_.max_index) + 1;
  }
  inline void Init(DataIter<Row<size_t> > *parser) {
    data_.Clear();
    while (parser->Next()) {
      data_.Push(parser->Value());
    }
    row_ = data_.GetBlock();
  }
  
 private:
  // at head
  bool at_head_;
  // maximum feature dimension
  size_t num_col;
  // row block to store
  RowBlock<IndexType> row_;
  // back end data
  RowBlockContainer<IndexType> data_;
};
}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_BASIC_ROW_ITER_H__
