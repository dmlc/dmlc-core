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

// Copyright by Contributors
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include <algorithm>
#include "./line_split.h"

namespace dmlc {
namespace io {
size_t LineSplitter::SeekRecordBegin(Stream *fi) {
  char c = '\0';
  size_t nstep = 0;
  // search till fist end-of-line
  while (true) {
    if (fi->Read(&c, sizeof(c)) == 0) return nstep;
    nstep += 1;
    if (c == '\n' || c == '\r') break;
  }
  // search until first non-endofline
  while (true) {
    if (fi->Read(&c, sizeof(c)) == 0) return nstep;
    if (c != '\n' && c != '\r') break;
    // non-end-of-line should not count
    nstep += 1;
  }
  return nstep;
}
const char* LineSplitter::FindLastRecordBegin(const char *begin,
                                              const char *end) {
  CHECK(begin != end);
  for (const char *p = end - 1; p != begin; --p) {
    if (*p == '\n' || *p == '\r') return p + 1;
  }
  return begin;
}

bool LineSplitter::ExtractNextRecord(Blob *out_rec, Chunk *chunk) {
  if (chunk->begin == chunk->end) return false;
  char *p;
  for (p = chunk->begin; p != chunk->end; ++p) {
    if (*p == '\n' || *p == '\r') break;
  }
  for (; p != chunk->end; ++p) {
    if (*p != '\n' && *p != '\r') break;
  }
  // set the string end sign for safety
  if (p == chunk->end) {
    *p = '\0';
  } else {
    *(p - 1) = '\0';
  }
  out_rec->dptr = chunk->begin;
  out_rec->size = p - chunk->begin;
  chunk->begin = p;
  return true;
}

}  // namespace io
}  // namespace dmlc
