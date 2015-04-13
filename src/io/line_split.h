/*!
 *  Copyright (c) 2015 by Contributors
 * \file line_split.h
 * \brief base class implementation of input splitter
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_LINE_SPLIT_H_
#define DMLC_IO_LINE_SPLIT_H_

#include <vector>
#include <cstdio>
#include <string>
#include <cstring>
#include <dmlc/io.h>
#include "./input_split_base.h"

namespace dmlc {
namespace io {
/*! \brief class that split the files by line */
class LineSplitter : public InputSplitBase {
 public:
  LineSplitter(FileSystem *fs,
               const char *uri,
               unsigned rank,
               unsigned nsplit) {
    this->Init(fs, uri, rank, nsplit, 1);
  }

 protected:
  void SeekRecordBegin(bool at_begin);
  bool NextRecord(std::string *out_data);
  /*!
   * \brief skip consecutive end of line marks, until
   *  only one end-of-line was left in bptr,
   *  or stop if there is not end of line at all
   */
  void SkipEndOfLines(void);
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_LINE_SPLIT_H_
