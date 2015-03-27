/*!
 *  Copyright (c) 2015 by Contributors
 * \file line_split.h
 * \brief base implementation of line-spliter
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_LINE_SPLIT_H_
#define DMLC_IO_LINE_SPLIT_H_

#include <string>
#include <cstring>
#include "../../include/io.h"
#include "../../include/io/stream_buffer_reader.h"

namespace dmlc {
namespace io {
/*! \brief class that split the files by line */
class LineSplitter : public InputSplit {
 public:
  /*! \brief provider class that provide  */
  class IFileProvider {
   public:
    /*!
     * \return const reference to size of each files
     */
    virtual const std::vector<size_t> &ListFileSize(void) const = 0;
    /*!
     * \brief get the seek stream of given file_index
     * \param file_index the index of the file in the group
     * \return the corresponding seek stream at head of the stream
     *  the seek stream's resource can be freed by calling delete 
     */
    virtual ISeekStream *Open(size_t file_index) = 0;
    // virtual destructor
    virtual ~IFileProvider() {}
  };
  // constructor
  explicit LineSplitter(IFileProvider *provider,
                        unsigned rank,
                        unsigned nsplit);
  // destructor
  virtual ~LineSplitter(void);
  // get next line
  virtual bool NextLine(std::string *out_data);
  /*!
   * \brief split names given a splitter
   * \param out_fname st
   * \param uri the input file list
   * \param dlm deliminetr
   */
  static void SplitNames(std::vector<std::string> *out_fname,
                         const char *uri,
                         const char *dlm) {
    std::string uri_ = uri;
    char *p = std::strtok(BeginPtr(uri_), dlm);
    while (p != NULL) {
      out_fname->push_back(std::string(p));
      p = std::strtok(NULL, dlm);
    }
  }
  
 private:
  /*! \brief FileProvider */
  IFileProvider *provider_;
  /*! \brief current input stream */
  ISeekStream *fs_;
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
  StreamBufferReader reader_;
  /*! \brief buffer size */
  const static size_t kBufferSize = 256;  
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_LINE_SPLIT_H_
