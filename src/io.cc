// use direct path for to save compile flags
#include <cstring>
#include "../include/io.h"
#include "../include/logging.h"
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
}  // namespace dmlc
