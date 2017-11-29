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

// test reading speed from a InputSplit
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <dmlc/io.h>
#include <dmlc/base.h>

int main(int argc, char *argv[]) {
  if (argc < 5) {
    printf("Usage: <libsvm> partid npart\n");
    return 0;
  }
  using namespace dmlc;
  InputSplit *split = InputSplit::Create(argv[1],
                                         atoi(argv[2]),
                                         atoi(argv[3]),
                                         "text");
  InputSplit::Blob blb;
  while (split->NextChunk(&blb)) {
    std::cout << std::string((char*)blb.dptr, blb.size);
  }
  delete split;
  return 0;
}

