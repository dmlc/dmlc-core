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
 * \file azure_filesys.h
 * \brief Azure access module
 * \author Mu Li
 */
#ifndef DMLC_IO_AZURE_FILESYS_H_
#define DMLC_IO_AZURE_FILESYS_H_

#include <vector>
#include <string>
#include "./filesys.h"

namespace dmlc {
namespace io {

/*! \brief Microsoft Azure Blob filesystem */
class AzureFileSystem : public FileSystem {
 public:
  virtual ~AzureFileSystem() {}

  virtual FileInfo GetPathInfo(const URI &path) { return FileInfo(); }

  virtual void ListDirectory(const URI &path, std::vector<FileInfo> *out_list);

  virtual Stream *Open(const URI &path, const char* const flag, bool allow_null) {
    return NULL;
  }

  virtual SeekStream *OpenForRead(const URI &path, bool allow_null) {
    return NULL;
  }

  /*!
   * \brief get a singleton of AzureFileSystem when needed
   * \return a singleton instance
   */
  inline static AzureFileSystem *GetInstance(void) {
    static AzureFileSystem instance;
    return &instance;
  }

 private:
  /*! \brief constructor */
  AzureFileSystem();

  /*! \brief Azure storage account name */
  std::string azure_account_;

  /*! \brief Azure storage account key */
  std::string azure_key_;
};

}  // namespace io
}  // namespace dmlc

#endif  // DMLC_IO_AZURE_FILESYS_H_
