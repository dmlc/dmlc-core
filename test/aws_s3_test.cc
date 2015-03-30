#include <cstdio>
#include <cstdlib>
#include "../src/io/aws_s3-inl.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: url\n");
    return 0;
  }
  const char *keyid = getenv("AWS_ACCESS_KEY_ID");
  const char *seckey = getenv("AWS_SECRET_ACCESS_KEY");
  dmlc::io::AWSS3Handle handle(keyid, seckey);
  std::vector<dmlc::io::AWSS3Handle::ObjectInfo> info;
  handle.ListObjects(argv[1], &info);
  for (size_t i = 0; i < info.size(); ++i) {
    printf("%s\t%lu\n", info[i].key.c_str(), info[i].size);
  }
  return 0;
}
