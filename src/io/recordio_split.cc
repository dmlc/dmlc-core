#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/recordio.h>
#include <dmlc/logging.h>
#include "./recordio_split.h"
namespace dmlc {
namespace io {
size_t RecordIOSplitter::SeekRecordBegin(void) {
  CHECK((reinterpret_cast<size_t>(bptr()) & 3) == 0)
      << "address not aligned";
  size_t nstep = 0;
  while (true) {
    const char *p;
    for (p = bptr(); p + 4 < bend(); p += 4) {
      unsigned v = *reinterpret_cast<const unsigned*>(p);
      if (v == RecordIOWriter::kMagic) {
        unsigned lrec = *reinterpret_cast<const unsigned*>(p + 4);
        unsigned cflag = RecordIOWriter::DecodeFlag(lrec);        
        if (cflag == 0 || cflag == 1) {
          nstep += p - bptr();
          this->set_bptr(p);
          return nstep;
        }
      }
    }
    nstep += p - bptr();
    this->set_bptr(p);
    if (!this->FillBuffer(bend() - p)) return nstep; 
  }
  return nstep;
}
}  // namespace io
}  // namespace dmlc
