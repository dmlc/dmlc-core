/*!
 *  Copyright (c) 2015 by Contributors
 * \file line_split.h
 * \brief base implementation of line-spliter
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_LINE_SPLIT_H_
#define DMLC_IO_LINE_SPLIT_H_

#include <vector>
#include <cstdio>
#include <string>
#include <cstring>
#include <dmlc/io.h>
#include "./filesys.h"

namespace dmlc {
namespace io {
/*! \brief class that split the files by line */
class LineSplitter : public InputSplit {
 public:
  // constructor
  explicit LineSplitter(IFileSystem *fs,
                        const char *uri,
                        unsigned rank,
                        unsigned nsplit);
  // destructor
  virtual ~LineSplitter(void);
  // get next line
  virtual bool ReadLine(std::string *out_data);
  
 protected:
  /*! \brief initialize information in files */
  void InitInputFileInfo(const char *uri);

 private:
  /*! \brief FileSystem */
  IFileSystem *filesys_;
  /*! \brief information about files */
  std::vector<FileInfo> files_;
  /*! \brief current input stream */
  IStream *fs_;
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
  /*! \brief buffer reader */
  dmlc::istream reader_;
  /*! \brief buffer size */
  const static size_t kBufferSize = 256;  
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_LINE_SPLIT_H_
