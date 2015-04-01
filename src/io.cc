// use direct path for to save compile flags
#include <cstring>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include "io/line_split.h"
#include "io/single_file_split.h"
#include "io/local_filesys.h"

#if DMLC_USE_HDFS
#include "io/hdfs_filesys.h"
#endif

#if DMLC_USE_S3
//#include "io/aws_s3-inl.h"
#endif

namespace dmlc {
InputSplit* InputSplit::Create(const char *uri,
                               unsigned part,
                               unsigned nsplit) {
  using namespace std;
  using namespace dmlc::io;
  if (!strcmp(uri, "stdin")) {
    return new SingleFileSplit(uri);
  }
  if (!strncmp(uri, "file://", 7)) {
    return new LineSplitter(new LocalFileSystem(), uri, part, nsplit);
  }
  if (!strncmp(uri, "hdfs://", 7)) {
#if DMLC_USE_HDFS
    return new LineSplitter(new HDFSFileSystem(), uri, part, nsplit);
#else
    Error("Please compile with DMLC_USE_HDFS=1");
#endif
  }
  if (!strncmp(uri, "s3://", 5)) {
#if DMLC_USE_HDFS
      //return new LineSplitter(new S3Provider(uri), part, nsplit);
#else
    Error("Please compile with DMLC_USE_S3=1");
#endif
  }
  return new LineSplitter(new LocalFileSystem(), uri, part, nsplit);
}

IStream *IStream::Create(const char *uri, const char * const flag) {
  using namespace std;
  using namespace dmlc::io;
  if (!strncmp(uri, "file://", 7)) {
    return LocalFileSystem().Open(URI(uri), flag);
  }
  if (!strncmp(uri, "hdfs://", 7)) {
#if DMLC_USE_HDFS
    return HDFSFileSystem().Open(URI(uri), flag);
#else
    Error("Please compile with DMLC_USE_HDFS=1");
#endif
  }

  if (!strncmp(uri, "s3://", 5)) {
#if DMLC_USE_S3
    //return S3FileSytem().Open(S3FileSytem::Path(uri), flag);
#else
    Error("Please compile with DMLC_USE_S3=1");
#endif
  }
  return LocalFileSystem().Open(URI(uri), flag);
}
}  // namespace dmlc
