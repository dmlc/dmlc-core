/*!
 *  Copyright (c) 2017 by Contributors
 * \file indexed_recordio_split.h
 * \brief input split that splits indexed recordio files
 * \author Przemyslaw Tredak
 */
#ifndef DMLC_IO_INDEXED_RECORDIO_SPLIT_H_
#define DMLC_IO_INDEXED_RECORDIO_SPLIT_H_

#include <dmlc/io.h>
#include <dmlc/recordio.h>
#include <vector>
#include <cstdio>
#include <string>
#include <cstring>
#include <map>
#include <random>
#include "./input_split_base.h"

namespace dmlc {
namespace io {
/*! \brief class that splits the recordIO file by record */
class IndexedRecordIOSplitter : public InputSplitBase {
 public:
  IndexedRecordIOSplitter(FileSystem *fs,
                          const char *uri,
                          const char *index_uri,
                          unsigned rank,
                          unsigned nsplit) {
    this->Init(fs, uri, 4);
    this->ReadIndexFile(fs, index_uri);
    this->ResetPartition(rank, nsplit);
    this->shuffle_ = false;
    this->batch_size_ = 256;
  }

  virtual bool ExtractNextRecord(Blob *out_rec, Chunk *chunk) override;
  virtual bool ReadChunk(void *buf, size_t *size) override;
  virtual bool NextChunk(Blob *out_chunk) override;
  virtual void BeforeFirst(void) override;
  virtual bool NextBatch(Blob *out_chunk, size_t n_records) override;
  virtual bool NextRecord(Blob *out_rec) override {
    while (!ExtractNextRecord(out_rec, &tmp_chunk_)) {
      if (!tmp_chunk_.Load(this, buffer_size_)) return false;
      ++current_index_;
    }
    return true;
  }
  void SetShuffle(bool shuffle);
  void SetRandomSeed(size_t seed) {
    rnd_.seed(kRandMagic + seed);
  }
  void SetBatchSize(int batch_size) {
    this->batch_size_ = batch_size;
  }

 protected:
  virtual size_t SeekRecordBegin(Stream *fi) override;
  virtual const char*
  FindLastRecordBegin(const char *begin, const char *end) override;
  virtual void ReadIndexFile(FileSystem *fs, const std::string& index_uri);
  virtual void ResetPartition(unsigned rank, unsigned nsplit) override;

  std::vector<std::pair<size_t, size_t> > index_;
  std::vector<size_t> permutation_;
  bool shuffle_;
  size_t current_index_;
  size_t index_begin_;
  size_t index_end_;
  size_t batch_size_;
  const int kRandMagic = 111;
  std::mt19937 rnd_;
  
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_RECORDIO_SPLIT_H_
