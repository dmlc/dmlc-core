#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/recordio.h>
#include <dmlc/logging.h>
#include "./recordio_split.h"
namespace dmlc {
namespace io {
size_t RecordIOSplitter::SeekRecordBegin(Stream *fi) {
  size_t nstep = 0;
  unsigned v, lrec; 
  while (true) {
    if (fi->Read(&v, sizeof(v)) == 0) return nstep;
    nstep += sizeof(v);
    if (v == RecordIOWriter::kMagic) {
      CHECK(fi->Read(&lrec, sizeof(lrec)) != 0)
            << "invalid record io format";
      nstep += sizeof(lrec);
      unsigned cflag = RecordIOWriter::DecodeFlag(lrec);        
      if (cflag == 0 || cflag == 1) break;
    }
  }
  return nstep;
}
}  // namespace io
}  // namespace dmlc
