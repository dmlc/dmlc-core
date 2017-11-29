/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

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
#include <dmlc/timer.h>
#include "./row_block.h"
#include "./parser.h"

namespace dmlc {
namespace data {
/*!
 * \brief basic set of row iterators that provides
 * \tparam IndexType the type of index we are using
 */
template<typename IndexType>
class BasicRowIter: public RowBlockIter<IndexType> {
 public:
  explicit BasicRowIter(Parser<IndexType> *parser)
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

 private:
  // at head
  bool at_head_;
  // row block to store
  RowBlock<IndexType> row_;
  // back end data
  RowBlockContainer<IndexType> data_;
  // initialize
  inline void Init(Parser<IndexType> *parser);
};

template<typename IndexType>
inline void BasicRowIter<IndexType>::Init(Parser<IndexType> *parser) {
  data_.Clear();
  double tstart = GetTime();
  size_t bytes_expect = 10UL << 20UL;
  while (parser->Next()) {
    data_.Push(parser->Value());
    double tdiff = GetTime() - tstart;
    size_t bytes_read  = parser->BytesRead();
    if (bytes_read >= bytes_expect) {
      bytes_read = bytes_read >> 20UL;
      LOG(INFO) << bytes_read << "MB read,"
                << bytes_read / tdiff << " MB/sec";
      bytes_expect += 10UL << 20UL;
    }
  }
  row_ = data_.GetBlock();
  double tdiff = GetTime() - tstart;
  LOG(INFO) << "finish reading at "
            << (parser->BytesRead() >> 20UL) / tdiff
            << " MB/sec";
}
}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_BASIC_ROW_ITER_H__
