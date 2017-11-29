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

#include <iostream>
#include <dmlc/io.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: <filename>\n");
    return 0;
  }
  {// output
    dmlc::Stream *fs = dmlc::Stream::Create(argv[1], "w");
    dmlc::ostream os(fs);
    os << "hello-world " << 1e-10<< std::endl;
    delete fs;
  }
  {// input
    std::string name;
    double data;
    dmlc::Stream *fs = dmlc::Stream::Create(argv[1], "r");
    dmlc::istream is(fs);
    is >> name >> data;
    std::cout << name << " " << data << std::endl;
    delete fs;
  }
  return 0;
}
