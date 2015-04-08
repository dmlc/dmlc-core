// use direct path for to save compile flags
#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include "io/line_split.h"
#include "io/single_file_split.h"
#include "io/filesys.h"
#include "io/local_filesys.h"

#if DMLC_USE_HDFS
#include "io/hdfs_filesys.h"
#endif

#if DMLC_USE_S3
#include "io/s3_filesys.h"
#endif

namespace dmlc {
namespace io {
FileSystem *FileSystem::Create(const std::string &protocol) {
  if (protocol == "file://" || protocol.length() == 0) {
    return new LocalFileSystem();
  }
  if (protocol == "hdfs://") {
#if DMLC_USE_HDFS
    return new HDFSFileSystem();
#else
    LOG(FATAL) << "Please compile with DMLC_USE_HDFS=1 to use hdfs";
#endif
  }
  if (protocol == "s3://") {
#if DMLC_USE_S3
    return new S3FileSystem();
#else
    LOG(FATAL) << "Please compile with DMLC_USE_HDFS=1 to use hdfs";
#endif
  }
  LOG(FATAL) << "unknown filesystem protocol " + protocol;
  return NULL;
}
} // namespace io

InputSplit* InputSplit::Create(const char *uri,
                               unsigned part,
                               unsigned nsplit) {
  using namespace std;
  using namespace dmlc::io;
  if (!strcmp(uri, "stdin")) {
    return new SingleFileSplit(uri);
  }
  URI path(uri);
  return new LineSplitter(FileSystem::Create(path.protocol), uri, part, nsplit);
}

Stream *Stream::Create(const char *uri, const char * const flag) {
  using namespace std;
  using namespace dmlc::io;
  URI path(uri);
  FileSystem *fs = FileSystem::Create(path.protocol);
  Stream *ret = fs->Open(path, flag);
  delete fs;
  return ret;
}
}  // namespace dmlc
