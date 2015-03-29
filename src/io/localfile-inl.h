/*!
 *  Copyright (c) 2015 by Contributors
 * \file file-inl.h
 * \brief local filesystem I/O
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_LOCALFILE_INL_H_
#define DMLC_IO_LOCALFILE_INL_H_

#include <string>
#include <vector>
#include <cstdio>
#include <dmlc/io.h>
#include "./line_split.h"

namespace dmlc {
namespace io {
/*! \brief implementation of file i/o stream */
class FileStream : public ISeekStream {
 public:
  explicit FileStream(const char *fname, const char *mode)
      : use_stdio(false) {
    using namespace std;
#ifndef DMLC_STRICT_CXX98_
    if (!strcmp(fname, "stdin")) {
      use_stdio = true; fp = stdin;
    }
    if (!strcmp(fname, "stdout")) {
      use_stdio = true; fp = stdout;
    }
#endif
    if (!strncmp(fname, "file://", 7)) fname += 7;
    if (!use_stdio) {
      std::string flag = mode;
      if (flag == "w") flag = "wb";
      if (flag == "r") flag = "rb";
      fp = fopen64(fname, flag.c_str());
      if (fp == NULL) {
        Error("FileStream: fail to open %s", fname);
      }
    }
  }
  virtual ~FileStream(void) {
    this->Close();
  }
  virtual size_t Read(void *ptr, size_t size) {
    return std::fread(ptr, 1, size, fp);
  }
  virtual void Write(const void *ptr, size_t size) {
    std::fwrite(ptr, size, 1, fp);
  }
  virtual void Seek(size_t pos) {
    std::fseek(fp, static_cast<long>(pos), SEEK_SET);
  }
  virtual size_t Tell(void) {
    return std::ftell(fp);
  }
  virtual bool AtEnd(void) const {
    return std::feof(fp) != 0;
  }
  inline void Close(void) {
    if (fp != NULL && !use_stdio) {
      std::fclose(fp); fp = NULL;
    }
  }

 private:
  std::FILE *fp;
  bool use_stdio;
};

/*! \brief line split from normal file system */
class FileProvider : public LineSplitter::IFileProvider {
 public:
  explicit FileProvider(const char *uri) {
    LineSplitter::SplitNames(&fnames_, uri, "#");
    std::vector<size_t> fsize;
    for (size_t  i = 0; i < fnames_.size(); ++i) {
      if (!std::strncmp(fnames_[i].c_str(), "file://", 7)) {
        std::string tmp = fnames_[i].c_str() + 7;
        fnames_[i] = tmp;        
      }
      size_t fz = GetFileSize(fnames_[i].c_str());
      if (fz != 0) {
        fsize_.push_back(fz);
      }
    }
  }
  // destrucor
  virtual ~FileProvider(void) {}  
  virtual ISeekStream *Open(size_t file_index) {
    //utils::Assert(file_index < fnames_.size(), "file index exceed bound"); 
    return new FileStream(fnames_[file_index].c_str(), "rb");
  }
  virtual const std::vector<size_t> &ListFileSize(void) const {
    return fsize_;
  }
 private:
  // file sizes
  std::vector<size_t> fsize_;
  // file names
  std::vector<std::string> fnames_;
  // get file size
  inline static size_t GetFileSize(const char *fname) {    
    std::FILE *fp = fopen64(fname, "rb");
    if (fp == NULL) {
      Error("FileStream: fail to open %s", fname);
    }
    // NOTE: fseek may not be good, but serves as ok solution
    std::fseek(fp, 0, SEEK_END);
    size_t fsize = static_cast<size_t>(std::ftell(fp));
    std::fclose(fp);
    return fsize;
  }
};

/*! \brief line split from single FILE */
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
      if (fp == NULL) {
        Error("FileStream: fail to open %s", fname);
      }
    }
    end_of_file_ = false;
  }
  virtual ~SingleFileSplit(void) {
    if (!use_stdin_) std::fclose(fp_);
  }
  virtual bool ReadLine(std::string *out_data) {
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
#endif  // DMLC_IO_FILE_INL_H_

