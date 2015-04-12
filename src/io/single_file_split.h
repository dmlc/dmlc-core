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
  explicit SingleFileSplit(const char *fname) {
    if (!std::strcmp(fname, "stdin")) {
#ifndef DMLC_STRICT_CXX98_
      use_stdin_ = true; fp_ = stdin;
#endif
    }
    if (!use_stdin_) {
      std::FILE *fp = fopen64(fname, "rb");
      CHECK (fp != NULL) << "SingleFileSplit: fail to open " << fname;
    }
    end_of_file_ = false;
  }
  virtual ~SingleFileSplit(void) {
    if (!use_stdin_) std::fclose(fp_);
  }
  virtual bool ReadRecord(std::string *out_data) {
    if (end_of_file_) return false;
    out_data->clear();
    while (true) {
      char c = std::fgetc(fp_);
      if (c == EOF) {
        end_of_file_ = true;
      }
      if (c != '\r' && c != '\n' && c != EOF) {
        *out_data += c;
      } else {
        if (out_data->length() != 0) return true;
        if (end_of_file_) return false;
      }
    }
    return false;
  }  
    
 private:
  std::FILE *fp_;
  bool use_stdin_;
  bool end_of_file_;
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_SINGLE_FILE_SPLIT_H_
