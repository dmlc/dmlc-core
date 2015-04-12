#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/logging.h>
#include "./line_split.h"
namespace dmlc {
namespace io {
void LineSplitter::SeekRecordBegin(void) {
  while (true) {
    for (const char *p = bptr(); p != bend(); ++p) {
      if (*p == '\n' || *p == '\r') {
        this->add_to_bptr(p - bptr()); return;
      }
    }
    this->add_to_bptr(bend() - bptr());
    if (!this->FillBuffer()) return;
  }
}

bool LineSplitter::NextRecord(std::string *out_data) {  
  out_data->clear();
  
  while (true) {
    const char *p;
    for (p = bptr(); p != bend(); ++p) {
      if (*p != '\r' && *p != '\n' && *p != EOF) break;
    }
    this->add_to_bptr(p - bptr());
    if (p == bend()) {
      if (!this->FillBuffer()) return false;
    } else {
      break;
    }
  }
  while (true) {
    const char *p;
    for (p = bptr(); p != bend(); ++p) {
      if (*p == '\r' || *p == '\n' || *p == EOF) break;
    }
    size_t n = p - bptr();
    if (n != 0) {
      size_t rlen = out_data->length();
      out_data->resize(rlen + n);
      std::memcpy(BeginPtr(*out_data) + rlen, bptr(), n);
      this->add_to_bptr(n);
    }
    if (p != bend()) return true;
    if (!this->FillBuffer()) return true;      
  }
  return true;
}
}  // namespace io
}  // namespace dmlc
