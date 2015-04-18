#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include "./line_split.h"
namespace dmlc {
namespace io {
size_t LineSplitter::SeekRecordBegin(Stream *fi) {
  char c;
  size_t nstep = 0;
  // search till fist end-of-line
  while (true) {
    if (fi->Read(&c, sizeof(c)) == 0) return nstep;
    nstep += 1;
    if (c == '\n' || c == '\r') break;
  }
  // search until first non-endofline
  while (true) {
    if (fi->Read(&c, sizeof(c)) == 0) return nstep;
    nstep += 1;
    if (c != '\n' && c != '\r') break; 
  }
  return nstep;
}
const char* LineSplitter::FindLastRecordBegin(const char *begin,
                                              const char *end) {
  CHECK(begin != end);
  for (const char *p = end - 1; p != begin; ++p) {
    if (*p == '\n' || *p == '\r') return p + 1; 
  }
  return begin;
}

char* LineSplitter::FindNextRecord(char *begin, char *end) {
  char *p;
  for (p = begin; p != end; ++p) {
    if (*p == '\n' || *p == '\r') break;
  }
  for (; p != end; ++p) {
    if (*p != '\n' && *p != '\r') return p;
  }
  return end;
}
}  // namespace io
}  // namespace dmlc
