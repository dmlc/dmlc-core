/*!
 *  Copyright (c) 2015 by Contributors
 * \file hdfs_filesys.h
 * \brief HDFS access module
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_S3_FILESYS_H_
#define DMLC_IO_S3_FILESYS_H_

#include "./filesys.h"

namespace dmlc {
namespace io {
/*! \brief AWS S3 filesystem */
class S3FileSystem : public FileSystem {
 public:
  /*! \brief constructor */
  S3FileSystem();
  /*! \brief destructor */
  virtual ~S3FileSystem() {}
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
   */  
  virtual Stream *Open(const URI &path, const char* const flag);
  /*!
   * \brief open a seekable stream for read
   * \param path the path to the file
   * \return the result stream
   */
  virtual SeekStream *OpenForRead(const URI &path);

 private:
  /*! \brief AWS access id */
  std::string aws_access_id_;
  /*! \brief AWS secret key */
  std::string aws_secret_key_;
};
}  // namespace io
}  // namespace dmlc
#endif 
