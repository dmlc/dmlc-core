/*!
 *  Copyright (c) 2015 by Contributors
 * \file input_split_base.h
 * \brief base class to construct input split from multiple files
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_INPUT_SPLIT_BASE_H_
#define DMLC_IO_INPUT_SPLIT_BASE_H_
#include <vector>
#include <cstdio>
#include <string>
#include <cstring>
#include <dmlc/io.h>
#include "./filesys.h"

namespace dmlc {
namespace io {
/*! \brief base class to construct input split from multiple files */
class InputSplitBase : public InputSplit {
 public:
  // destructor
  virtual ~InputSplitBase(void);  
  /*!
   * \brief read a chunk of data into buf
   *   the data can span multiple records,
   *   but cannot contain partial records
   *
   * \param buf the memory region of the buffer,
   *        should be properly aligned to 64 bits
   * \param size the maximum size of memory,
   *   after the function returns, it stores the size of the chunk
   * \return whether end of file was reached
   */
  bool ReadChunk(void *buf, size_t *size);
  
 protected:
  // constructor 
  InputSplitBase()
      : fs_(NULL) {}
  /*!
   * \brief intialize the base before doing anything
   * \param fs the filesystem ptr
   * \param uri the uri of the files
   * \param rank the rank of the split
   * \param nsplit number of splits
   * \param align_bytes the head split must be multiple of align_bytes
   *   this also checks if file size are multiple of align_bytes
   */
  void Init(FileSystem *fs,
            const char *uri,
            unsigned rank,
            unsigned nsplit,
            size_t align_bytes);
  // to be implemented by child class
  /*!
   * \brief seek to the beginning of the first record
   *        in current file pointer
   * \return how many bytes we read past
   */
  virtual size_t SeekRecordBegin(Stream *fi) = 0;
  /*!
   * \brief find the last occurance of record
   *        in the dataset
   * \param begin beginning of the buffer
   * \param end end of the buffer
   * \return how many bytes we read past
   */
  virtual const char*
  FindLastRecordBegin(const char *begin, const char *end) = 0;

 private:
  /*! \brief FileSystem */
  FileSystem *filesys_;
  /*! \brief information about files */
  std::vector<FileInfo> files_;
  /*! \brief current input stream */
  SeekStream *fs_;
  /*! \brief file pointer of which file to read on */
  size_t file_ptr_;
  /*! \brief file pointer where the end of file lies */
  size_t file_ptr_end_;
  /*! \brief get the current offset */
  size_t offset_curr_;
  /*! \brief beginning of offset */
  size_t offset_begin_;
  /*! \brief end of the offset */
  size_t offset_end_;
  /*! \brief byte-offset of each file */
  std::vector<size_t> file_offset_;
  /*! \brief internal overflow buffer */
  std::string overflow_;
  /*! \brief initialize information in files */
  void InitInputFileInfo(const char *uri);
  /*! \brief same as stream.Read */
  size_t Read(void *ptr, size_t size);  
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_LINE_SPLIT_H_
