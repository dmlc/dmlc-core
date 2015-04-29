#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/recordio.h>
#include <dmlc/logging.h>
#include "./recordio_split.h"
namespace dmlc {
namespace io {
size_t RecordIOSplitter::SeekRecordBegin(Stream *fi) {
  size_t nstep = 0;
  uint32_t v, lrec; 
  while (true) {
    if (fi->Read(&v, sizeof(v)) == 0) return nstep;
    nstep += sizeof(v);
    if (v == RecordIOWriter::kMagic) {
      CHECK(fi->Read(&lrec, sizeof(lrec)) != 0)
            << "invalid record io format";
      nstep += sizeof(lrec);
      uint32_t cflag = RecordIOWriter::DecodeFlag(lrec);        
      if (cflag == 0 || cflag == 1) break;
    }
  }
  // should point at head of record
  return nstep - 2 * sizeof(uint32_t);
}
const char* RecordIOSplitter::FindLastRecordBegin(const char *begin,
                                                  const char *end) {
  CHECK((reinterpret_cast<size_t>(begin) & 3UL) == 0); 
  CHECK((reinterpret_cast<size_t>(end) & 3UL) == 0);
  const uint32_t *pbegin = reinterpret_cast<const uint32_t *>(begin);
  const uint32_t *p = reinterpret_cast<const uint32_t *>(end);
  CHECK(p >= pbegin + 2);
  for (p = p - 2; p != pbegin; --p) {
    if (p[0] == RecordIOWriter::kMagic) {
      uint32_t cflag = RecordIOWriter::DecodeFlag(p[1]);
      if (cflag == 0 || cflag == 1) {
        return reinterpret_cast<const char*>(p);
      }
    }
  }
  return begin;
}

char* RecordIOSplitter::FindNextRecord(char *begin, char *end) {
  CHECK((reinterpret_cast<size_t>(begin) & 3UL) == 0); 
  CHECK((reinterpret_cast<size_t>(end) & 3UL) == 0);
  while (true) {
    uint32_t *p = reinterpret_cast<uint32_t *>(begin);
    uint32_t cflag = RecordIOWriter::DecodeFlag(p[1]);
    uint32_t clen = RecordIOWriter::DecodeLength(p[1]);
    begin += 2 * sizeof(uint32_t) + (((clen + 3U) >> 2U) >> 2U);
    if (cflag == 0 || cflag == 2) return begin;
    CHECK(begin <= end) << "invalid chunk";
  }
  return end;
}
}  // namespace io
}  // namespace dmlc
