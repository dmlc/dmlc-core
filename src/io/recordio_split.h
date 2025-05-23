/*!
 *  Copyright (c) 2015 by Contributors
 * \file recordio_split.h
 * \brief input split that splits recordio files
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_RECORDIO_SPLIT_H_
#define DMLC_IO_RECORDIO_SPLIT_H_

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <dmlc/io.h>
#include <dmlc/recordio.h>

#include "./input_split_base.h"

namespace dmlc {
namespace io {
/*! \brief class that split the files by line */
class RecordIOSplitter : public InputSplitBase {
 public:
  RecordIOSplitter(FileSystem *fs, const char *uri, unsigned rank, unsigned nsplit,
      const bool recurse_directories) {
    this->Init(fs, uri, 4, recurse_directories);
    this->ResetPartition(rank, nsplit);
  }

  bool IsTextParser(void) {
    return false;
  }
  virtual bool ExtractNextRecord(Blob *out_rec, Chunk *chunk);

 protected:
  virtual size_t SeekRecordBegin(Stream *fi);
  virtual const char *FindLastRecordBegin(const char *begin, const char *end);
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_RECORDIO_SPLIT_H_
