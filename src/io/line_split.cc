#include <algorithm>
#include <dmlc/logging.h>
#include "./line_split.h"

namespace dmlc {
namespace io {
LineSplitter::LineSplitter(IFileProvider *provider,
                           unsigned rank,
                           unsigned nsplit) 
  : provider_(provider), fs_(NULL),
    reader_(kBufferSize) {
  std::vector<size_t> file_size = provider->ListFileSize();  
  file_offset_.resize(file_size.size() + 1);
  file_offset_[0] = 0;
  for (size_t i = 0; i < file_size.size(); ++i) {
    file_offset_[i + 1] = file_offset_[i] + file_size[i];
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
  fs_ = provider_->Open(file_ptr_, offset_begin_ - file_offset_[file_ptr_]);
  reader_.set_stream(fs_);
  // try to set the starting position correctly
  if (file_offset_[file_ptr_] != offset_begin_) {
    while (true) {
      char c = reader_.GetChar(); 
      if (!reader_.AtEnd()) ++offset_curr_;
      if (c == '\n' || c == '\r' || c == EOF) return;
    }
  }  
}

bool LineSplitter::ReadLine(std::string *out_data) {
  if (file_ptr_ >= file_ptr_end_ &&
      offset_curr_ >= offset_end_) return false;
  out_data->clear();
  while (true) {
    char c = reader_.GetChar();
    if (reader_.AtEnd()) {
      if (out_data->length() != 0) return true;
      file_ptr_ += 1;
      if (offset_curr_ >= offset_end_) return false;
      if (offset_curr_ != file_offset_[file_ptr_]) {
        LOG(FATAL) <<"FILE size not calculated correctly\n";
        offset_curr_ = file_offset_[file_ptr_];
      }
      CHECK(file_ptr_ + 1 < file_offset_.size()) << "boundary check";
      delete fs_;
      fs_ = provider_->Open(file_ptr_, 0);
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
