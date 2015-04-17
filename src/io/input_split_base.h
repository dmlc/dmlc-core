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
  // disable write
  virtual void Write(const void *buf, size_t size) {
    LOG(FATAL) << "InputSplit do not support write";
  }
  // override read
  virtual size_t Read(void *ptr, size_t size);  
  // destructor
  virtual ~InputSplitBase(void);
  
 protected:
  // constructor 
  InputSplitBase()
      : fs_(NULL), buffer_(kBufferSize),
        bptr_(NULL), bend_(NULL) {}
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
  /*!
   * \brief fill the buffer with current input stream
   * \param bytes_kept number of bytes that will be kept at buffer head
   *   this should be bptr() - bend(), caller must give this to double
   *   check the request is consistent
   * \return true if future bytes are loaded in,
   *    false if no future bytes are loaded
   */
  bool FillBuffer(size_t bytes_kept = 0);
  /*! \return buffer current pointer */
  inline const char *bptr(void) const {
    return bptr_;
  }
  /*! \return buffer end pointer */
  inline const char *bend(void) const {
    return bend_;
  }
  /*!
   * \brief set bptr
   * \param bptr to set 
   */
  inline void set_bptr(const char *bptr) {
    bptr_ = bptr;
  }
  // to be implemented by child class
  /*!
   * \brief seek to the beginning of the first record
   * in current file pointer
   * \return how many bytes we read past
   */
  virtual size_t SeekRecordBegin(void) = 0;

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
  /*! \brief buffer size */
  const static size_t kBufferSize = 1024;
  /*! \brief internal buffer */
  std::vector<size_t> buffer_;
  /*! \brief internal buffer pointer */
  const char *bptr_;
  /*! \brief internal buffer end pointer */
  const char *bend_;  
  /*! \brief initialize information in files */
  void InitInputFileInfo(const char *uri);
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_LINE_SPLIT_H_
