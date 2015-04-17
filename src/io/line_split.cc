#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/logging.h>
#include "./line_split.h"
namespace dmlc {
namespace io {
size_t LineSplitter::SeekRecordBegin(void) {
  size_t nstep = 0;
  // search till fist end-of-line
  while (true) {
    const char *p;
    for (p = bptr(); p != bend(); ++p) {
      if (*p == '\n' || *p == '\r') break;
    }
    nstep += p - bptr();
    this->set_bptr(p);
    if (p != bend()) break;
    if (!this->FillBuffer()) return nstep;
  }
  while (true) {
    const char *p;
    for (p = bptr(); p != bend(); ++p) {
      if (*p != '\n' || *p != '\r') break;
    }
    nstep += p - bptr();
    this->set_bptr(p);
    if (p != bend()) break;
    if (!this->FillBuffer()) return nstep;    
  }
  return nstep;
}
}  // namespace io
}  // namespace dmlc
