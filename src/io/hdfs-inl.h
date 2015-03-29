/*!
 *  Copyright (c) 2015 by Contributors
 * \file hdfs-inl.h
 * \brief HDFS I/O code
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_HDFS_INL_H_
#define DMLC_IO_HDFS_INL_H_

#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <hdfs.h>
#include <errno.h>
#include <dmlc/io.h>
#include "./line_split.h"

/*! \brief io interface */
namespace dmlc {
namespace io {
class HDFSStream : public ISeekStream {
 public:
  HDFSStream(hdfsFS fs,
             const char *fname,
             const char *mode,
             bool disconnect_when_done)
      : fs_(fs), at_end_(false),
        disconnect_when_done_(disconnect_when_done) {
    int flag = 0;
    if (!strcmp(mode, "r")) {
      flag = O_RDONLY;
    } else if (!strcmp(mode, "w"))  {
      flag = O_WRONLY;
    } else if (!strcmp(mode, "a"))  {
      flag = O_WRONLY | O_APPEND;
    } else {
      Error("HDFSStream: unknown flag %s", mode);
    }
    fp_ = hdfsOpenFile(fs_, fname, flag, 0, 0, 0);
    if (fp_ == NULL) {
      Error("HDFSStream: fail to open %s", fname);
    }
  }
  virtual ~HDFSStream(void) {
    this->Close();
    if (disconnect_when_done_) {
      if (hdfsDisconnect(fs_) != 0) {
        int errsv = errno;
        Error("HDFSStream.hdfsDisconnect Error:%s", strerror(errsv));
      }
    }
  }
  virtual size_t Read(void *ptr, size_t size) {
    tSize nread = hdfsRead(fs_, fp_, ptr, size);
    if (nread == -1) {
      int errsv = errno;
      Error("HDFSStream.Read Error:%s", strerror(errsv));
    }
    if (nread == 0) {
      at_end_ = true;
    }
    return static_cast<size_t>(nread);
  }
  virtual void Write(const void *ptr, size_t size) {
    const char *buf = reinterpret_cast<const char*>(ptr);
    while (size != 0) {
      tSize nwrite = hdfsWrite(fs_, fp_, buf, size);
      if (nwrite == -1) {
        int errsv = errno;
        Error("HDFSStream.Write Error:%s", strerror(errsv));
      }
      size_t sz = static_cast<size_t>(nwrite);
      buf += sz; size -= sz;
    }
  }
  virtual void Seek(size_t pos) {
    if (hdfsSeek(fs_, fp_, pos) != 0) {
      int errsv = errno;
      Error("HDFSStream.Seek Error:%s", strerror(errsv));
    }
  }
  virtual size_t Tell(void) {
    tOffset offset = hdfsTell(fs_, fp_);
    if (offset == -1) {
      int errsv = errno;
      Error("HDFSStream.Tell Error:%s", strerror(errsv));
    }
    return static_cast<size_t>(offset);
  }
  virtual bool AtEnd(void) const {
    return at_end_;
  }
  inline void Close(void) {
    if (fp_ != NULL) {
      if (hdfsCloseFile(fs_, fp_) == -1) {
        int errsv = errno;
        Error("HDFSStream.Close Error:%s", strerror(errsv));
      }
      fp_ = NULL;
    }
  }  
  inline static std::string GetNameNode(void) {
    return std::string("default");
  }
 private:
  hdfsFS fs_;
  hdfsFile fp_;
  bool at_end_;
  bool disconnect_when_done_;
};

/*! \brief line split from normal file system */
class HDFSProvider : public LineSplitter::IFileProvider {
 public:
  explicit HDFSProvider(const char *uri) {
    fs_ = hdfsConnect(HDFSStream::GetNameNode().c_str(), 0);
    if (fs_ == NULL) {
      int errsv = errno;      
      Error("error when connecting to HDFS:%s", strerror(errsv));
    }
    std::vector<std::string> paths;
    LineSplitter::SplitNames(&paths, uri, "#");
    // get the files
    for (size_t  i = 0; i < paths.size(); ++i) {
      hdfsFileInfo *info = hdfsGetPathInfo(fs_, paths[i].c_str());
      if (info == NULL) {
        Error("path %s do not exist", paths[i].c_str());
      }
      if (info->mKind == 'D') {
        int nentry;
        hdfsFileInfo *files = hdfsListDirectory(fs_, info->mName, &nentry);
        if (files == NULL) {
          Error("error when ListDirectory %s", info->mName);
        }
        for (int i = 0; i < nentry; ++i) {
          if (files[i].mKind == 'F' && files[i].mSize != 0) {
            fsize_.push_back(files[i].mSize);            
            fnames_.push_back(std::string(files[i].mName));
          }
        }
        hdfsFreeFileInfo(files, nentry);
      } else {
        if (info->mSize != 0) {
          fsize_.push_back(info->mSize);
          fnames_.push_back(std::string(info->mName));
        }
      }
      hdfsFreeFileInfo(info, 1);
    }
  }
  virtual ~HDFSProvider(void) {
    if (hdfsDisconnect(fs_) != 0) {
      int errsv = errno;
      Error("HDFSStream.hdfsDisconnect Error:%s", strerror(errsv));
    }
  }  
  virtual const std::vector<size_t> &ListFileSize(void) const {
    return fsize_;
  }
  virtual ISeekStream *Open(size_t file_index) {
    //utils::Assert(file_index < fnames_.size(), "file index exceed bound"); 
    return new HDFSStream(fs_, fnames_[file_index].c_str(), "r", false);
  }
  
 private:
  // hdfs handle
  hdfsFS fs_;
  // file sizes
  std::vector<size_t> fsize_;
  // file names
  std::vector<std::string> fnames_;
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_HDFS_INL_H_
