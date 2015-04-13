#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <dmlc/recordio.h>
#include <dmlc/logging.h>
#include "./recordio_split.h"
namespace dmlc {
namespace io {
void RecordIOSplitter::SeekRecordBegin(bool at_begin) {
  CHECK((reinterpret_cast<size_t>(bptr()) & 3) == 0)
      << "address not aligned";
  while (true) {
    const char *p;
    for (p = bptr(); p + 4 < bend(); p += 4) {
      unsigned v = *reinterpret_cast<const unsigned*>(p);
      if (v == RecordIOWriter::kMagic) {
        unsigned lrec = *reinterpret_cast<const unsigned*>(p + 4);
        unsigned cflag = RecordIOWriter::DecodeFlag(lrec);
        if (cflag == 0 || cflag == 1) {
          this->add_to_bptr(p - bptr()); return;
        }
      }
    }
    this->add_to_bptr(p - bptr());
    if (!this->FillBuffer(bend() - p)) return;    
  }
}

bool RecordIOSplitter::NextRecord(std::string *out_data) {
  const unsigned kMagic = RecordIOWriter::kMagic;
  out_data->clear();
  size_t size = 0;

  while (true) {
    if (bptr() + 4 >= bend()) {
      if (!this->FillBuffer(bend() - bptr())) return false;      
    }
    // read in header
    const char *p = bptr();
    unsigned v = *reinterpret_cast<const unsigned*>(p);
    CHECK(v == RecordIOWriter::kMagic) << "Invalid RecordIO Format";
    unsigned lrec = *reinterpret_cast<const unsigned*>(p + 4);
    unsigned cflag = RecordIOWriter::DecodeFlag(lrec);
    unsigned len = RecordIOWriter::DecodeLength(lrec);
    size_t target_size = size + len;
    // read bytes are ligned with 4 bytes
    size_t nread = ((len + 3U) >> 2U) << 2U;
    this->add_to_bptr(8);    
    // read in data
    out_data->resize(size + nread);
    while (nread != 0) {
      size_t diff = bend() - bptr();
      size_t ncopy = std::min(diff, nread);
      std::memcpy(BeginPtr(*out_data) + size, bptr(), ncopy);
      nread -= ncopy; size += ncopy;
      this->add_to_bptr(ncopy);
      if (nread != 0) {
        CHECK(this->FillBuffer()) << "Invalid RecordIO Format";
      }
    }
    // squeeze back to target size to ignore padding bytes
    out_data->resize(size = target_size);
    if (cflag == 0U || cflag == 3U) return true;  
    out_data->resize(size + sizeof(kMagic));
    std::memcpy(BeginPtr(*out_data) + size, &kMagic, sizeof(kMagic));
    size += sizeof(kMagic);
  }
  LOG(FATAL) << "cannot reach here";
  return false;
}
}  // namespace io
}  // namespace dmlc
