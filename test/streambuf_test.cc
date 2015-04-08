#include <iostream>
#include <dmlc/io.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: <filename>\n");
    return 0;
  }
  dmlc::IStream *fs = dmlc::IStream::Create(argv[1], "w");
  dmlc::StreamBuf buf(fs);
  std::ostream os(&buf);
  os << "hello world" << std::endl;
  delete fs;
    
  return 0;
}
