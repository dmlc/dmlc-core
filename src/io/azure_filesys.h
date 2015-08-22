/*!
 *  Copyright (c) 2015 by Contributors
 * \file azure_filesys.h
 * \brief Azure access module
 * \author Kamran Munshi
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

  virtual FileInfo GetPathInfo(const URI &path);

  virtual void ListDirectory(const URI &path, std::vector<FileInfo> *out_list);

  virtual Stream *Open(const URI &path, const char* flag, bool allow_null);

  virtual SeekStream *OpenForRead(const URI &path, bool allow_null);

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
  
  bool TryGetPathInfo(const URI &path, FileInfo *info);
};

}  // namespace io
}  // namespace dmlc

#endif  // DMLC_IO_AZURE_FILESYS_H_
