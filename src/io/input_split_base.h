/*!
 *  Copyright (c) 2015 by Contributors
 * \file input_split_base.h
 * \brief base class to construct input split from multiple files
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_INPUT_SPLIT_BASE_H_
#define DMLC_IO_INPUT_SPLIT_BASE_H_

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <dmlc/filesystem.h>
#include <dmlc/io.h>

namespace dmlc {
namespace io {
/*! \brief class to construct input split from multiple files */
class InputSplitBase : public InputSplit {
 public:
  /*!
   * \brief helper struct to hold chunk data
   *  with internal pointer to move along the record
   */
  struct Chunk {
    char *begin;
    char *end;
    std::vector<uint32_t> data;
    explicit Chunk(size_t buffer_size) : begin(NULL), end(NULL), data(buffer_size + 1) {}
    // load chunk from split
    bool Load(InputSplitBase *split, size_t buffer_size);
    // append to chunk
    bool Append(InputSplitBase *split, size_t buffer_size);
  };
  // 16 MB
  static const size_t kBufferSize = 2UL << 20UL;
  // destructor
  virtual ~InputSplitBase(void);
  // implement BeforeFirst
  virtual void BeforeFirst(void);
  virtual void HintChunkSize(size_t chunk_size) {
    buffer_size_ = std::max(chunk_size / sizeof(uint32_t), buffer_size_);
  }
  virtual size_t GetTotalSize(void) {
    return file_offset_.back();
  }
  // implement next record
  virtual bool NextRecord(Blob *out_rec) {
    while (!ExtractNextRecord(out_rec, &tmp_chunk_)) {
      if (!NextChunkEx(&tmp_chunk_)) {
        return false;
      }
    }
    return true;
  }
  // implement next chunk
  virtual bool NextChunk(Blob *out_chunk) {
    while (!ExtractNextChunk(out_chunk, &tmp_chunk_)) {
      if (!NextChunkEx(&tmp_chunk_)) {
        return false;
      }
    }
    return true;
  }
  // implement ResetPartition.
  virtual void ResetPartition(unsigned rank, unsigned nsplit);
  /*!
   * \brief read a chunk of data into buf
   *   the data can span multiple records,
   *   but cannot contain partial records
   *
   * \param buf the memory region of the buffer,
   *        should be properly aligned to 64 bits
   * \param size the maximum size of memory,
   *   after the function returns, it stores the size of the chunk
   * \return whether end of file was reached
   */
  virtual bool ReadChunk(void *buf, size_t *size);
  /*!
   * \brief extract next chunk from the chunk
   * \param out_chunk the output record
   * \param chunk the chunk information
   * \return true if non-empty record is extracted
   *    false if the chunk is already finishes its life
   */
  bool ExtractNextChunk(Blob *out_rchunk, Chunk *chunk);
  /*!
   * \brief extract next record from the chunk
   * \param out_rec the output record
   * \param chunk the chunk information
   * \return true if non-empty record is extracted
   *    false if the chunk is already finishes its life
   */
  virtual bool ExtractNextRecord(Blob *out_rec, Chunk *chunk) = 0;
  /*!
   * \brief query whether this object is a text parser
   * \return true if this object represents a text parser; false if it represents
   *         a binary parser
   */
  virtual bool IsTextParser(void) = 0;
  /*!
   * \brief fill the given
   *  chunk with new data without using internal
   *  temporary chunk
   */
  virtual bool NextChunkEx(Chunk *chunk) {
    if (!chunk->Load(this, buffer_size_)) {
      return false;
    }
    return true;
  }
  /*!
   * \brief fill the given
   *  chunk with new batch of data without using internal
   *  temporary chunk
   */
  virtual bool NextBatchEx(Chunk *chunk, size_t /*n_records*/) {
    return NextChunkEx(chunk);
  }

 protected:
  /*! \brief FileSystem */
  FileSystem *filesys_;
  /*! \brief byte-offset of each file */
  std::vector<size_t> file_offset_;
  /*! \brief get the current offset */
  size_t offset_curr_;
  /*! \brief beginning of offset */
  size_t offset_begin_;
  /*! \brief end of the offset */
  size_t offset_end_;
  /*! \brief information about files */
  std::vector<FileInfo> files_;
  /*! \brief current input stream */
  SeekStream *fs_;
  /*! \brief file pointer of which file to read on */
  size_t file_ptr_;
  /*! \brief file pointer where the end of file lies */
  size_t file_ptr_end_;
  /*! \brief temporal chunk */
  Chunk tmp_chunk_;
  /*! \brief buffer size */
  size_t buffer_size_;
  // constructor
  InputSplitBase()
      : fs_(NULL), tmp_chunk_(kBufferSize), buffer_size_(kBufferSize), align_bytes_(8) {}
  /*!
   * \brief intialize the base before doing anything
   * \param fs the filesystem ptr
   * \param uri the uri of the files
   * \param rank the rank of the split
   * \param nsplit number of splits
   * \param align_bytes the head split must be multiple of align_bytes
   *   this also checks if file size are multiple of align_bytes
   * \param recurse_directories recursively travese directories
   */
  void Init(
      FileSystem *fs, const char *uri, size_t align_bytes, const bool recurse_directories = false);
  // to be implemented by child class
  /*!
   * \brief seek to the beginning of the first record
   *        in current file pointer
   * \return how many bytes we read past
   */
  virtual size_t SeekRecordBegin(Stream *fi) = 0;
  /*!
   * \brief find the last occurance of record header
   * \param begin beginning of the buffer
   * \param end end of the buffer
   * \return the pointer between [begin, end] indicating the
   *         last record head
   */
  virtual const char *FindLastRecordBegin(const char *begin, const char *end) = 0;

  /*! \brief split string list of files into vector of URIs */
  std::vector<URI> ConvertToURIs(const std::string &uri);
  /*! \brief same as stream.Read */
  size_t Read(void *ptr, size_t size);

 private:
  /*! \brief bytes to be aligned */
  size_t align_bytes_;
  /*! \brief internal overflow buffer */
  std::string overflow_;
  /*! \brief initialize information in files */
  void InitInputFileInfo(const std::string &uri, const bool recurse_directories);
  /*! \brief strip continous chars in the end of str */
  std::string StripEnd(std::string str, char ch);
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_INPUT_SPLIT_BASE_H_
