/*!
 *  Copyright (c) 2015 by Contributors  
 * \file buffer_reader.h
 * \brief implementation of stream buffer reader
 */
#ifndef DMLC_IO_STREAM_BUFFER_READER_H_
#define DMLC_IO_STREAM_BUFFER_READER_H_
#include <dmlc/io.h>

namespace dmlc {
/*! \brief namespace of all things related to io */
namespace io {
/*! \brief buffer reader of the stream that allows you to getchar */
class StreamBufferReader {
 public:
  StreamBufferReader(size_t buffer_size)
      :stream_(NULL),
       read_len_(1), read_ptr_(1) {
    buffer_.resize(buffer_size);
  }
  /*!
   * \brief set input stream
   */
  inline void set_stream(IStream *stream) {
    stream_ = stream;
    read_len_ = read_ptr_ = 1;
  }
  /*!
   * \brief allows quick read using get char
   */
  inline char GetChar(void) {
    while (true) {
      if (read_ptr_ < read_len_) {
        return buffer_[read_ptr_++];
      } else {
        read_len_ = stream_->Read(&buffer_[0], buffer_.length());
        if (read_len_ == 0) return EOF;
        read_ptr_ = 0;
      }
    }
  }
  /*! \brief whether we are reaching the end of file */
  inline bool AtEnd(void) const {
    return read_len_ == 0;
  }
  
 private:
  /*! \brief the underlying stream */
  IStream *stream_;
  /*! \brief buffer to hold data */
  std::string buffer_;
  /*! \brief length of valid data in buffer */
  size_t read_len_;
  /*! \brief pointer in the buffer */
  size_t read_ptr_;
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_BUFFER_READER_H_
