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
  if (offset_begin_ != file_offset_[file_ptr_]) {
    this->SeekRecordBegin();
  }
}

InputSplitBase::~InputSplitBase(void) {
  if (fs_ != NULL) {
    delete fs_; fs_ = NULL;
  }
  delete filesys_;  
}

void InputSplitBase::InitInputFileInfo(const char *uri) {
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

bool InputSplitBase::ReadRecord(std::string *out_data) {
  if (file_ptr_ >= file_ptr_end_ &&
      offset_curr_ >= offset_end_) return false;
  while (true) {
    if (this->NextRecord(out_data)) return true;
    file_ptr_ += 1;
    if (offset_curr_ >= offset_end_) return false;
      if (offset_curr_ != file_offset_[file_ptr_]) {
        LOG(FATAL) <<"FILE size not calculated correctly\n";
        offset_curr_ = file_offset_[file_ptr_];
      }
      CHECK(file_ptr_ + 1 < file_offset_.size()) << "boundary check";
      delete fs_;
      fs_ = filesys_->OpenPartForRead(files_[file_ptr_].path, 0);
  }
  return false;
}

bool InputSplitBase::FillBuffer(void) {
  CHECK(bptr_ == bend_)
      << "only call fillbuffer when buffer is already readed";
  bptr_ = reinterpret_cast<const char*>(BeginPtr(buffer_));
  size_t n = fs_->Read(BeginPtr(buffer_),
                       buffer_.size() * sizeof(size_t));
  bend_ = bptr_ + n;
  return n != 0;
}
}  // namespace io
}  // namespace dmlc
