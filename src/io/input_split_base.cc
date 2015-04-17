#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/logging.h>
#include "./line_split.h"

namespace dmlc {
namespace io {
void InputSplitBase::Init(FileSystem *filesys,
                          const char *uri,
                          unsigned rank,
                          unsigned nsplit,
                          size_t align_bytes) {
  this->filesys_ = filesys;
  // initialize the path
  this->InitInputFileInfo(uri);  
  file_offset_.resize(files_.size() + 1);
  file_offset_[0] = 0;
  for (size_t i = 0; i < files_.size(); ++i) {
    file_offset_[i + 1] = file_offset_[i] + files_[i].size;
    CHECK(files_[i].size % align_bytes == 0)
        << "file do not align by " << align_bytes << " bytes";
  }
  size_t ntotal = file_offset_.back();
  size_t nstep = (ntotal + nsplit - 1) / nsplit;
  // align the nstep to 4 bytes
  nstep = ((nstep + align_bytes - 1) / align_bytes) * align_bytes;
  offset_begin_ = std::min(nstep * rank, ntotal);
  offset_end_ = std::min(nstep * (rank + 1), ntotal);
  if (offset_begin_ == offset_end_) return;
  file_ptr_ = std::upper_bound(file_offset_.begin(),
                               file_offset_.end(),
                               offset_begin_) - file_offset_.begin() - 1;
  file_ptr_end_ = std::upper_bound(file_offset_.begin(),
                                   file_offset_.end(),
                                   offset_end_) - file_offset_.begin() - 1;
  
  // find the exact ending position
  if (offset_end_ != file_offset_[file_ptr_end_]) {
    CHECK(offset_end_ >file_offset_[file_ptr_end_]);
    CHECK(file_ptr_end_ < files_.size());
    fs_ = filesys_->OpenForRead(files_[file_ptr_end_].path);
    fs_->Seek(offset_end_ - file_offset_[file_ptr_end_]);
    offset_end_ += SeekRecordBegin();
    delete fs_;
  }
  fs_ = filesys_->OpenForRead(files_[file_ptr_].path); 
  if (offset_begin_ != file_offset_[file_ptr_]) {
    fs_->Seek(offset_begin_ - file_offset_[file_ptr_]);
    offset_curr_ = offset_begin_ + SeekRecordBegin();
    // seek to beginning of stream
    fs_->Seek(offset_curr_ - file_offset_[file_ptr_]);
  }
  // reset buffer
  bptr_ = bend_ = NULL;
}

InputSplitBase::~InputSplitBase(void) {
  if (fs_ != NULL) {
    delete fs_; fs_ = NULL;
  }
  delete filesys_;  
}

void InputSplitBase::InitInputFileInfo(const char *uri) {
  // split by :
  const char *dlm = ":";
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

size_t InputSplitBase::Read(void *ptr, size_t size) {
  if (offset_curr_ +  size > offset_end_) {
    size = offset_end_ - offset_curr_;
  }
  if (size == 0) return 0;
  size_t nleft = size;
  char *buf = reinterpret_cast<char*>(ptr);
  while (true) {
    size_t n = fs_->Read(buf, nleft);
    nleft -= n; buf += n;
    offset_curr_ += n;
    if (nleft == 0) break;
    file_ptr_ += 1;
    if (offset_curr_ != file_offset_[file_ptr_]) {
      LOG(FATAL) << "FILE size not calculated correctly\n";
    }
    if (file_ptr_ >= files_.size()) break;
    delete fs_;
    fs_ = filesys_->OpenForRead(files_[file_ptr_].path);    
  }
  return size - nleft;
}

bool InputSplitBase::FillBuffer(size_t bytes_kept) {
  CHECK(bptr_ + bytes_kept == bend_)
      << "inconsistent FillBuffer request";
  char *bhead = reinterpret_cast<char*>(BeginPtr(buffer_));  
  if (bytes_kept != 0) {
    std::memmove(bhead, bptr_, bytes_kept);    
  }  
  bptr_ = bhead;
  size_t nread = buffer_.size() * sizeof(size_t) - bytes_kept;
  size_t n = fs_->Read(bhead + bytes_kept, nread);
  bend_ = bptr_ + n + bytes_kept;
  return n != 0;
}
}  // namespace io
}  // namespace dmlc
