#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/logging.h>
#include "./line_split.h"
namespace dmlc {
namespace io {
void LineSplitter::SkipEndOfLines(void) {
  if (bptr() == bend()) {
    if (!this->FillBuffer()) return;
  }
  if (*bptr() != '\n' && *bptr() != '\r') return;  
  while (true) {
    const char *p;
    for (p = bptr(); p + 1 < bend(); ++p) {
      if (p[1] != '\n' && p[1] != '\r' && p[1] != EOF) {
        this->add_to_bptr(p - bptr()); return;
      }
    }
    this->add_to_bptr(p - bptr()); return;
    if (!this->FillBuffer(1)) return;
  }
}
void LineSplitter::SeekRecordBegin(bool at_begin) {
  // search till fist end-of-line
  if (!at_begin) {
    while (true) {
      const char *p;
      for (p = bptr(); p != bend(); ++p) {
        if (*p == '\n' || *p == '\r') break;
      }
      this->add_to_bptr(p - bptr());
      if (p != bend()) break;
      if (!this->FillBuffer()) return;
    }
  }
  
  // skip extra endof lines
  this->SkipEndOfLines();
}

bool LineSplitter::NextRecord(std::string *out_data) {  
  out_data->clear();
  // strip all the end-of-lines
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
  // read until next end of line
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
    if (p != bend()) break;
    if (!this->FillBuffer()) break;
  }
  // skip additional endof lines
  this->SkipEndOfLines();
  return true;
}
}  // namespace io
}  // namespace dmlc
