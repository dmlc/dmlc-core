// use direct path for to save compile flags
#include <cstring>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include "io/line_split.h"
#include "io/localfile-inl.h"

#if DMLC_USE_HDFS
#include "io/hdfs-inl.h"
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
    return new LineSplitter(new FileProvider(uri), part, nsplit);
  }
  if (!strncmp(uri, "hdfs://", 7)) {
#if DMLC_USE_HDFS
    return new LineSplitter(new HDFSProvider(uri), part, nsplit);
#else
    Error("Please compile with DMLC_USE_HDFS=1");
#endif
  }
  return new LineSplitter(new FileProvider(uri), part, nsplit);
}

ISeekStream *ISeekStream::Create(const char *uri, const char * const flag) {
  using namespace std;
  using namespace dmlc::io;
  if (!strncmp(uri, "file://", 7)) {
    return new FileStream(uri + 7, flag);
  }
  if (!strncmp(uri, "hdfs://", 7)) {
#if DMLC_USE_HDFS
    return new HDFSStream(hdfsConnect(HDFSStream::GetNameNode().c_str(), 0),
                          uri, flag, true);
#else
    Error("Please compile with DMLC_USE_HDFS=1");
#endif
  }
  return new FileStream(uri, flag);
}
}  // namespace dmlc
