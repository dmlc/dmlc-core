#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/logging.h>
#include "./line_split.h"

namespace dmlc {
namespace io {
LineSplitter::LineSplitter(FileSystem *filesys,
                           const char *uri,
                           unsigned rank,
                           unsigned nsplit) 
  : filesys_(filesys), fs_(NULL),
    reader_(NULL, kBufferSize) {
  // initialize the path
  this->InitInputFileInfo(uri);
  
  file_offset_.resize(files_.size() + 1);
  file_offset_[0] = 0;
  for (size_t i = 0; i < files_.size(); ++i) {
    file_offset_[i + 1] = file_offset_[i] + files_[i].size;
  }
  size_t ntotal = file_offset_.back();
  size_t nstep = (ntotal + nsplit - 1) / nsplit;
  offset_begin_ = std::min(nstep * rank, ntotal);
  offset_end_ = std::min(nstep * (rank + 1), ntotal);    
  offset_curr_ = offset_begin_;
  if (offset_begin_ == offset_end_) return;
  file_ptr_ = std::upper_bound(file_offset_.begin(),
                               file_offset_.end(),
                               offset_begin_) - file_offset_.begin() - 1;
  file_ptr_end_ = std::upper_bound(file_offset_.begin(),
                                   file_offset_.end(),
                                   offset_end_) - file_offset_.begin() - 1;
  fs_ = filesys_->OpenPartForRead(files_[file_ptr_].path,
                                  offset_begin_ - file_offset_[file_ptr_]);
  reader_.set_stream(fs_);
  // try to set the starting position correctly
  if (file_offset_[file_ptr_] != offset_begin_) {
    while (true) {
      char c = reader_.get(); 
      if (!reader_.eof()) ++offset_curr_;
      if (c == '\n' || c == '\r' || c == EOF) return;
    }
  }  
}

LineSplitter::~LineSplitter(void) {
  if (fs_ != NULL) {
    delete fs_; fs_ = NULL;
  }
  delete filesys_;  
}

void LineSplitter::InitInputFileInfo(const char *uri) {
  // split by #
  const char *dlm = "#";
  std::string uri_ = uri;
  char *p = std::strtok(BeginPtr(uri_), dlm);
  std::vector<URI> vec;

  while (p != NULL) {
    URI path(p);
    FileInfo info = filesys_->GetPathInfo(path);
    if (info.type == kDirectory) {
      std::vector<FileInfo> dfiles;
      filesys_->ListDirectory(info.path, &dfiles);
      for (size_t i = 0; i < dfiles.size(); ++i) {
        if (dfiles[i].size != 0 && dfiles[i].type == kFile) {
          files_.push_back(dfiles[i]);
        }
      }
    } else {
      if (info.size != 0) {
        files_.push_back(info);
      }
    }
    p = std::strtok(NULL, dlm);
  }
}

bool LineSplitter::ReadRecord(std::string *out_data) {
  if (file_ptr_ >= file_ptr_end_ &&
      offset_curr_ >= offset_end_) return false;
  out_data->clear();
  while (true) {
    char c = reader_.get();
    if (reader_.eof()) {
      if (out_data->length() != 0) return true;
      file_ptr_ += 1;
      if (offset_curr_ >= offset_end_) return false;
      if (offset_curr_ != file_offset_[file_ptr_]) {
        LOG(FATAL) <<"FILE size not calculated correctly\n";
        offset_curr_ = file_offset_[file_ptr_];
      }
      CHECK(file_ptr_ + 1 < file_offset_.size()) << "boundary check";
      delete fs_;
      fs_ = filesys_->OpenPartForRead(files_[file_ptr_].path, 0);
      reader_.set_stream(fs_);
    } else {
      ++offset_curr_;
      if (c != '\r' && c != '\n' && c != EOF) {
        *out_data += c;
      } else {
        if (out_data->length() != 0) return true;
        if (file_ptr_ >= file_ptr_end_ &&
            offset_curr_ >= offset_end_) return false;
      }
    }
  }  
}
}  // namespace io
}  // namespace dmlc
