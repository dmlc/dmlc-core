/*!
 *  Copyright (c) 2015 by Contributors
 * \file hdfs_filesys.h
 * \brief HDFS access module
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_HDFS_FILESYS_H_
#define DMLC_IO_HDFS_FILESYS_H_

#include <hdfs.h>
#include "./filesys.h"

namespace dmlc {
namespace io {
/*! \brief HDFS file system */
class HDFSFileSystem : public IFileSystem {
 public:
  /*! \brief constructor */
  HDFSFileSystem();
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
   * NOTE: the IStream can continue to work even when filesystem was destructed
   * \param path path to file
   * \param uri the uri of the input, can contain hdfs prefix
   * \param flag can be "w", "r", "a"   
   */  
  virtual IStream *Open(const URI &path, const char* const flag);
  /*!
   * \brief open a part of stream stream for read,
   *   with ability to specify starting location
   * \param path the path to the file
   * \parma begin_bytes the beginning bytes to start reading
   */
  virtual IStream *OpenPartForRead(const URI &path, size_t begin_bytes);

 private:
  /*! \brief namenode address */
  std::string namenode_;
  /*! \brief hdfs handle */
  hdfsFS fs_;
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_HDFS_FILESYS_H_
