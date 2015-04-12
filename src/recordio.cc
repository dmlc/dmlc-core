// use direct path for to save compile flags
#define _CRT_SECURE_NO_WARNINGS
#include <dmlc/base.h>
#include <dmlc/recordio.h>
#include <dmlc/logging.h>

namespace dmlc {
// implemmentation
void RecordIOWriter::WriteRecord(const void *buf, size_t size) {
  CHECK(size < (1 << 29U))
      << "RecordIO only accept record less than 2^29 bytes"; 
  const unsigned umagic = kMagic;
  // initialize the magic number, in stack
  const char *magic = reinterpret_cast<const char*>(&umagic);
  const char *bhead = reinterpret_cast<const char*>(buf);
  unsigned len = static_cast<unsigned>(size);
  unsigned lower_align = (len >> 2U) << 2U;
  unsigned upper_align = ((len + 3U) >> 2U) << 2U;
  unsigned dptr = 0;
  for (unsigned i = 0; i < lower_align ; i += 4) {
    // use char check for alignment safety reason
    if (bhead[i] == magic[0] &&
        bhead[i + 1] == magic[1] &&
        bhead[i + 2] == magic[2] &&
        bhead[i + 3] == magic[3]) {
      unsigned lrec = EncodeLRec(dptr == 0 ? 1U : 2U,
                                 i - dptr);
      stream_->Write(magic, 4);
      stream_->Write(&lrec, sizeof(lrec));
      if (i != dptr) {
        stream_->Write(bhead + dptr, i - dptr);
      }
      dptr = i + 4;
      except_counter_ += 1;
    }
  }
  unsigned lrec = EncodeLRec(dptr != 0 ? 3U : 0U,
                             len - dptr);
  stream_->Write(magic, 4);
  stream_->Write(&lrec, sizeof(lrec));
  if (len != dptr) {
    stream_->Write(bhead + dptr, len - dptr);
  }
  // write padded bytes
  unsigned zero = 0;
  if (upper_align != len) {
    stream_->Write(&zero, upper_align - len);
  }
}

bool RecordIOReader::ReadRecord(std::string *out_rec) {
  if (end_of_stream_) return false;
  const unsigned kMagic = RecordIOWriter::kMagic;
  out_rec->clear();
  size_t size = 0;
  while (true) {
    unsigned header[2];
    size_t nread = stream_->Read(header, sizeof(header));
    if (nread == 0) {
      end_of_stream_ = true; return false;
    }
    CHECK(nread == sizeof(header)) << "Inavlid RecordIO File";
    CHECK(header[0] == RecordIOWriter::kMagic) << "Invalid RecordIO File";
    unsigned cflag = RecordIOWriter::DecodeFlag(header[1]);
    unsigned len = RecordIOWriter::DecodeLength(header[1]);
    unsigned upper_align = ((len + 3U) >> 2U) << 2U;
    out_rec->resize(size + upper_align);
    if (upper_align != 0) {
      CHECK(stream_->Read(BeginPtr(*out_rec) + size, upper_align) == upper_align)
          << "Invalid RecordIO File upper_align=" << upper_align;
    }
    // squeeze back
    size += len; out_rec->resize(size);
    if (cflag == 0U || cflag == 3U) break;
    out_rec->resize(size + sizeof(kMagic));
    std::memcpy(BeginPtr(*out_rec) + size, &kMagic, sizeof(kMagic));
    size += sizeof(kMagic);
  }
  return true;
}
}  // namespace dmlc
