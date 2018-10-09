/*!
 *  Copyright (c) 2018 by Contributors
 * \file filesystem.h
 * \brief Utilities to manipulate files
 * \author Hyunsu Philip Cho
 */
#ifndef DMLC_FILESYSTEM_H_
#define DMLC_FILESYSTEM_H_

#include <dmlc/logging.h>
#include <string>
#include <vector>

/* platform specific headers */
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#else
#include <unistd.h>
#endif

namespace dmlc {

/*!
 * \brief Manager class for temporary directories. Whenever a new object is
 *        constructed, a temporary directory is created. The directory is
 *        deleted when the object is deleted or goes out of scope.
 *        NOTE. The class needs to keep track of all files inside the temporary
 *        directory in order to delete the directory successfully in the end.
 *        Thus, the user should call AddFile() method to add each file.
 */
class TemporaryDirectory {
 public:
  /*!
   * \brief Default constructor. Creates a new temporary directory
   * \param verbose whether to emit extra messages
   */
  explicit TemporaryDirectory(bool verbose = false)
    : verbose_(verbose) {
#if _WIN32
    /* locate the root directory of temporary area */
    char tmproot[MAX_PATH] = {0};
    const DWORD dw_retval = GetTempPathA(MAX_PATH, tmproot);
    if (dw_retval > MAX_PATH || dw_retval == 0) {
      LOG(FATAL) << "TemporaryDirectory(): "
                 << "Could not create temporary directory";
    }
    /* generate a unique 8-letter alphanumeric string */
    const std::string letters = "abcdefghijklmnopqrstuvwxyz0123456789_";
    std::string uniqstr(8, '\0');
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, letters.length() - 1);
    std::generate(uniqstr.begin(), uniqstr.end(),
      [&dis, &gen, &letters]() -> char {
        return letters[dis(gen)];
      });
    /* combine paths to get the name of the temporary directory */
    char tmpdir[MAX_PATH] = {0};
    PathCombineA(tmpdir, tmproot, uniqstr.c_str());
    if (!CreateDirectoryA(tmpdir, NULL)) {
      LOG(FATAL) << "TemporaryDirectory(): "
                 << "Could not create temporary directory";
    }
    path = std::string(tmpdir);
#else
    std::string tmproot; /* root directory of temporary area */
    std::string dirtemplate; /* template for temporary directory name */
    /* Get TMPDIR env variable or fall back to /tmp/ */
    {
      const char* tmpenv = getenv("TMPDIR");
      if (tmpenv) {
        tmproot = std::string(tmpenv);
        // strip trailing forward slashes
        while (tmproot.length() != 0 && tmproot[tmproot.length() - 1] == '/') {
          tmproot.resize(tmproot.length() - 1);
        }
      } else {
        tmproot = "/tmp";
      }
    }
    dirtemplate = tmproot + "/tmpdir.XXXXXX";
    std::vector<char> dirtemplate_buf(dirtemplate.begin(), dirtemplate.end());
    dirtemplate_buf.push_back('\0');
    char* tmpdir = mkdtemp(&dirtemplate_buf[0]);
    if (!tmpdir) {
      LOG(FATAL) << "TemporaryDirectory(): "
                 << "Could not create temporary directory";
    }
    path = std::string(tmpdir);
#endif
    if (verbose_) {
      LOG(INFO) << "Created temporary directory " << path;
    }
  }

  ~TemporaryDirectory() {
    for (const std::string& filename : file_list_) {
      if (std::remove(filename.c_str()) != 0) {
        LOG(FATAL) << "Couldn't remove file " << filename;
      }
    }
#if _WIN32
    const bool rmdir_success = (RemoveDirectoryA(path.c_str()) != 0);
#else
    const bool rmdir_success = (rmdir(path.c_str()) == 0);
#endif
    if (rmdir_success) {
      if (verbose_) {
        LOG(INFO) << "Successfully deleted temporary directory " << path;
      }
    } else {
      LOG(FATAL) << "~TemporaryDirectory(): "
                 << "Could not remove temporary directory " << path;
    }
  }

  /*!
   * \brief Add a file under the temporary directory, to be tracked. When the
   *        directory gets deleted, the file will get deleted also.
   * \param filename name of the file
   * \return full path of the file
   */
  std::string AddFile(const std::string& filename) {
    const std::string file_path = this->path + "/" + filename;
    file_list_.push_back(file_path);
    return file_path;
  }

  /*! \brief Path of the temporary directory */
  std::string path;

 private:
  /*! \brief List of files being tracked  */
  std::vector<std::string> file_list_;
  /*! \brief Whether to emit extra messages */
  bool verbose_;
};

}  // namespace dmlc
#endif  // DMLC_FILESYSTEM_H_
