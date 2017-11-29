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
 * \file text_parser.h
 * \brief iterator parser to parse text format
 * \author Tianqi Chen
 */
#ifndef DMLC_DATA_TEXT_PARSER_H_
#define DMLC_DATA_TEXT_PARSER_H_

#include <dmlc/data.h>
#include <dmlc/omp.h>
#include <vector>
#include <cstring>
#include <algorithm>
#include "./row_block.h"
#include "./parser.h"

namespace dmlc {
namespace data {
/*!
 * \brief Text parser that parses the input lines
 * and returns rows in input data
 */
template <typename IndexType>
class TextParserBase : public ParserImpl<IndexType> {
 public:
  explicit TextParserBase(InputSplit *source,
                          int nthread)
      : bytes_read_(0), source_(source) {
    int maxthread;
    #pragma omp parallel
    {
      maxthread = std::max(omp_get_num_procs() / 2 - 4, 1);
    }
    nthread_ = std::min(maxthread, nthread);
  }
  virtual ~TextParserBase() {
    delete source_;
  }
  virtual void BeforeFirst(void) {
    source_->BeforeFirst();
  }
  virtual size_t BytesRead(void) const {
    return bytes_read_;
  }
  virtual bool ParseNext(std::vector<RowBlockContainer<IndexType> > *data) {
    return FillData(data);
  }

 protected:
  /*!
   * \brief parse data into out
   * \param begin beginning of buffer
   * \param end end of buffer
   */
  virtual void ParseBlock(char *begin,
                          char *end,
                          RowBlockContainer<IndexType> *out) = 0;
  /*!
   * \brief read in next several blocks of data
   * \param data vector of data to be returned
   * \return true if the data is loaded, false if reach end
   */
  inline bool FillData(std::vector<RowBlockContainer<IndexType> > *data);
  /*!
   * \brief start from bptr, go backward and find first endof line
   * \param bptr end position to go backward
   * \param begin the beginning position of buffer
   * \return position of first endof line going backward
   */
  inline char* BackFindEndLine(char *bptr,
                               char *begin) {
    for (; bptr != begin; --bptr) {
      if (*bptr == '\n' || *bptr == '\r') return bptr;
    }
    return begin;
  }

 private:
  // nthread
  int nthread_;
  // number of bytes readed
  size_t bytes_read_;
  // source split that provides the data
  InputSplit *source_;
};

// implementation
template <typename IndexType>
inline bool TextParserBase<IndexType>::
FillData(std::vector<RowBlockContainer<IndexType> > *data) {
  InputSplit::Blob chunk;
  if (!source_->NextChunk(&chunk)) return false;
  const int nthread = omp_get_max_threads();
  // reserve space for data
  data->resize(nthread);
  bytes_read_ += chunk.size;
  CHECK_NE(chunk.size, 0U);
  char *head = reinterpret_cast<char*>(chunk.dptr);
  #pragma omp parallel num_threads(nthread)
  {
    // threadid
    int tid = omp_get_thread_num();
    size_t nstep = (chunk.size + nthread - 1) / nthread;
    size_t sbegin = std::min(tid * nstep, chunk.size);
    size_t send = std::min((tid + 1) * nstep, chunk.size);
    char *pbegin = BackFindEndLine(head + sbegin, head);
    char *pend;
    if (tid + 1 == nthread) {
      pend = head + send;
    } else {
      pend = BackFindEndLine(head + send, head);
    }
    ParseBlock(pbegin, pend, &(*data)[tid]);
  }
  this->data_ptr_ = 0;
  return true;
}

}  // namespace data
}  // namespace dmlc
#endif  // DMLC_DATA_TEXT_PARSER_H_
