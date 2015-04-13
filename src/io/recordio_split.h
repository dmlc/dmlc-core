/*!
 *  Copyright (c) 2015 by Contributors
 * \file recordio_split.h
 * \brief input split that splits recordio files
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_RECORDIO_SPLIT_H_
#define DMLC_IO_RECORDIO_SPLIT_H_

#include <vector>
#include <cstdio>
#include <string>
#include <cstring>
#include <dmlc/io.h>
#include <dmlc/recordio.h>
#include "./input_split_base.h"

namespace dmlc {
namespace io {
/*! \brief class that split the files by line */
class RecordIOSplitter : public InputSplitBase {
 public:
  RecordIOSplitter(FileSystem *fs,
                   const char *uri,
                   unsigned rank,
                   unsigned nsplit) {
    this->Init(fs, uri, rank, nsplit, 4);
  }

 protected:
  void SeekRecordBegin(bool at_begin);
  bool NextRecord(std::string *out_data);   
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_RECORDIO_SPLIT_H_
