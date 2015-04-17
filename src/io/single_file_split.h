/*!
 *  Copyright (c) 2015 by Contributors
 * \file single_file_split.h
 * \brief base implementation of line-spliter
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_SINGLE_FILE_SPLIT_H_
#define DMLC_IO_SINGLE_FILE_SPLIT_H_
#include <cstdio>
#include <dmlc/io.h>
#include <dmlc/logging.h>

namespace dmlc {
namespace io {
/*!
 * \brief line split implementation from single FILE 
 * simply returns lines of files, used for stdin
 */
class SingleFileSplit : public InputSplit {
 public:
  explicit SingleFileSplit(const char *fname)
      : use_stdin_(false) {
    if (!std::strcmp(fname, "stdin")) {
#ifndef DMLC_STRICT_CXX98_
      use_stdin_ = true; fp_ = stdin;
#endif
    }
    if (!use_stdin_) {
      fp_ = fopen64(fname, "rb");
      CHECK (fp_ != NULL) << "SingleFileSplit: fail to open " << fname;
    }
  }
  virtual ~SingleFileSplit(void) {
    if (!use_stdin_) std::fclose(fp_);
  }
  virtual size_t Read(void *ptr, size_t size) {
    return std::fread(ptr, 1, size, fp_);
  }
  virtual void Write(const void *ptr, size_t size) {
    LOG(FATAL) << "InputSplit do not support write";
  }  
  
 private:
  std::FILE *fp_;
  bool use_stdin_;
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_SINGLE_FILE_SPLIT_H_
