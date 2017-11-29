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
 * \file hdfs_filesys.h
 * \brief HDFS access module
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_HDFS_FILESYS_H_
#define DMLC_IO_HDFS_FILESYS_H_
extern "C" {
#include <hdfs.h>
}
#include <vector>
#include <string>
#include "./filesys.h"

namespace dmlc {
namespace io {
/*! \brief HDFS file system */
class HDFSFileSystem : public FileSystem {
 public:
  /*! \brief destructor */
  virtual ~HDFSFileSystem();
  /*!
   * \brief get information about a path
   * \param path the path to the file
   * \return the information about the file
   */
  virtual FileInfo GetPathInfo(const URI &path);
  /*!
   * \brief list files in a directory
   * \param path to the file
   * \param out_list the output information about the files
   */
  virtual void ListDirectory(const URI &path, std::vector<FileInfo> *out_list);
  /*!
   * \brief open a stream, will report error and exit if bad thing happens
   * NOTE: the Stream can continue to work even when filesystem was destructed
   * \param path path to file
   * \param uri the uri of the input, can contain hdfs prefix
   * \param flag can be "w", "r", "a"
   * \param allow_null whether NULL can be returned, or directly report error
   * \return the created stream, can be NULL when allow_null == true and file do not exist
   */
  virtual SeekStream *Open(const URI &path,
                           const char* const flag,
                           bool allow_null);
  /*!
   * \brief open a seekable stream for read
   * \param path the path to the file
   * \param allow_null whether NULL can be returned, or directly report error
   * \return the created stream, can be NULL when allow_null == true and file do not exist
   */
  virtual SeekStream *OpenForRead(const URI &path, bool allow_null);
  /*!
   * \brief get a singleton of HDFSFileSystem when needed
   * \return a singleton instance
   */
  inline static HDFSFileSystem *GetInstance(const std::string &namenode = "default") {
    static HDFSFileSystem instance(namenode);
    // switch to another hdfs
    if (namenode != "default" && instance.namenode_ != namenode) {
      instance.ResetNamenode(namenode);
    }
    return &instance;
  }

 private:
  /*! \brief constructor */
  explicit HDFSFileSystem(const std::string &namenode);
  /*! \brief switch to another hdfs cluster */
  void ResetNamenode(const std::string &namenode);
  /*! \brief namenode address */
  std::string namenode_;
  /*! \brief hdfs handle */
  hdfsFS fs_;
  /*! \brief reference counter of fs */
  int *ref_counter_;
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_HDFS_FILESYS_H_
