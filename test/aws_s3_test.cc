#include <cstdio>
#include <cstdlib>

#include "../src/io/aws_s3-inl.h"
#include "../src/io/stream_buffer_reader.h"
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: url\n");
    return 0;
  }
  using namespace dmlc::io;
  const char *keyid = getenv("AWS_ACCESS_KEY_ID");
  const char *seckey = getenv("AWS_SECRET_ACCESS_KEY");
  dmlc::io::S3FileSytem handle(keyid, seckey);
std::vector<S3FileSytem::ObjectInfo> info;
  handle.ListPath(S3FileSytem::Path(argv[1]), &info);
  for (size_t i = 0; i < info.size(); ++i) {
    printf("%s\t%lu\tis_dir=%d\n", info[i].key.c_str(), info[i].size,
           info[i].is_dir);
  }
  if (argc > 2) {
    dmlc::IStream *fs = handle.OpenForRead(S3FileSytem::Path(argv[2]), 0);
    StreamBufferReader reader(64);
    reader.set_stream(fs);
    //while (!reader.AtEnd()) {
    //fputc(reader.GetChar(), stdout);
    //}
    char buf[32];
    while (true) {
      size_t nread = fs->Read(buf, 32);
      if (nread == 0) break;
      fprintf(stderr, "%s", std::string(buf, nread).c_str());
    }
    fflush(stderr);
    delete fs;
  }
  return 0;
}
