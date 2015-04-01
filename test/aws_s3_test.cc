#include <cstdio>
#include <cstdlib>

#include "../src/io/s3_filesys.h"
#include "../src/io/stream_buffer_reader.h"
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: url\n");
    return 0;
  }
  using namespace dmlc::io;

  dmlc::io::S3FileSystem fs;
  std::vector<FileInfo> info;
  fs.ListDirectory(URI(argv[1]), &info);
  for (size_t i = 0; i < info.size(); ++i) {
    printf("%s\t%lu\tis_dir=%d\n", info[i].path.name.c_str(), info[i].size,
           info[i].type == kDirectory);
  }
  if (argc > 2) {
    dmlc::IStream *fp = fs.OpenPartForRead(URI(argv[2]), 0);
    char buf[32];
    while (true) {
      size_t nread = fp->Read(buf, 32);
      if (nread == 0) break;
      fprintf(stderr, "%s", std::string(buf, nread).c_str());
    }
    fflush(stderr);
    delete fp;
  }
  return 0;
}
