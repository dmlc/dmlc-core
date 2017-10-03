// Copyright by Contributors
#include <queue>

#include "./filesys.h"

namespace dmlc {
namespace io {

void FileSystem::ListDirectoryRecursive(const URI &path,
                                        std::vector<FileInfo> *out_list) {
  std::queue<URI> queue;
  queue.push(path);
  while (!queue.empty()) {
    std::vector<FileInfo> dfiles;
    ListDirectory(queue.front(), &dfiles);
    queue.pop();
    for (auto dfile : dfiles) {
      if (dfile.type == kDirectory) {
        queue.push(dfile.path);
      } else {
        out_list->push_back(dfile);
      }
    }
  }
}

}  // namespace io
}  // namespace dmlc
