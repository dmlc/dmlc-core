#define _CRT_SECURE_NO_WARNINGS
#include <dmlc/logging.h>
#include "./local_filesys.h"
#include <errno.h>
extern "C"{
#include <sys/stat.h>
}
#ifndef _MSC_VER
extern "C" {
#include <sys/types.h>
#include <dirent.h>
}
#else
#include <Windows.h>
#endif

namespace dmlc {
namespace io {
/*! \brief implementation of file i/o stream */
class FileStream : public ISeekStream {
 public:
  explicit FileStream(const char *fname, const char *mode)
      : use_stdio(false) {
    using namespace std;
#ifndef DMLC_STRICT_CXX98_
    if (!strcmp(fname, "stdin")) {
      use_stdio = true; fp = stdin;
    }
    if (!strcmp(fname, "stdout")) {
      use_stdio = true; fp = stdout;
    }
#endif
    if (!strncmp(fname, "file://", 7)) fname += 7;
    if (!use_stdio) {
      std::string flag = mode;
      if (flag == "w") flag = "wb";
      if (flag == "r") flag = "rb";
      fp = fopen64(fname, flag.c_str());
      if (fp == NULL) {
        Error("FileStream: fail to open %s", fname);
      }
    }
  }
  virtual ~FileStream(void) {
    this->Close();
  }
  virtual size_t Read(void *ptr, size_t size) {
    return std::fread(ptr, 1, size, fp);
  }
  virtual void Write(const void *ptr, size_t size) {
    std::fwrite(ptr, size, 1, fp);
  }
  virtual void Seek(size_t pos) {
    std::fseek(fp, static_cast<long>(pos), SEEK_SET);
  }
  virtual size_t Tell(void) {
    return std::ftell(fp);
  }
  virtual bool AtEnd(void) const {
    return std::feof(fp) != 0;
  }
  inline void Close(void) {
    if (fp != NULL && !use_stdio) {
      std::fclose(fp); fp = NULL;
    }
  }

 private:
  std::FILE *fp;
  bool use_stdio;
};

FileInfo LocalFileSystem::GetPathInfo(const URI &path) {
  struct stat sb;
  if (stat(path.name.c_str(), &sb) == -1) {
    int errsv = errno;
    Error("LocalFileSystem.GetPathInfo %s error:%s", path.name.c_str(), strerror(errsv));
  }
  FileInfo ret;
  ret.path = path;
  ret.size = sb.st_size;

  if ((sb.st_mode & S_IFMT) == S_IFDIR) {
    ret.type = kDirectory;
  } else {
    ret.type = kFile;
  }
  return ret;
}

void LocalFileSystem::ListDirectory(const URI &path, std::vector<FileInfo> *out_list) {
#ifndef _MSC_VER
  DIR *dir = opendir(path.name.c_str());
  if (dir == NULL) {
    int errsv = errno;
    Error("LocalFileSystem.ListDirectory  error: %s", strerror(errsv));
  }
  out_list->clear();
  struct dirent *ent;
  /* print all the files and directories within directory */
  while ((ent = readdir(dir)) != NULL) {
    if (!strcmp(ent->d_name, ".")) continue;
    if (!strcmp(ent->d_name, "..")) continue;
    URI pp = path;
    if (pp.name[pp.name.length() - 1] != '/') {
      pp.name += '/';
    }
    pp.name += ent->d_name;
    out_list->push_back(GetPathInfo(pp));
  }
  closedir(dir);
#else
  WIN32_FIND_DATA fd;
  std::string pattern = path.name + "/*";
  HANDLE handle = FindFirstFile(pattern.c_str(), &fd); 
  if (handle == INVALID_HANDLE_VALUE) {
    int errsv = GetLastError();
    Error("LocalFileSystem.ListDirectory %s error: %s", path.name.c_str(), strerror(errsv));  
  }
  do {
   if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")) {
    URI pp = path;
	char clast = pp.name[pp.name.length() - 1];
	if (pp.name == ".") {
	  pp.name = fd.cFileName;
	} else if (clast != '/' && clast != '\\') {
      pp.name += '/';
	  pp.name += fd.cFileName;
    }
	out_list->push_back(GetPathInfo(pp));
   }
  }  while(FindNextFile(handle, &fd)); 
  FindClose(handle); 
#endif
}

ISeekStream *LocalFileSystem::Open(const URI &path, const char* const flag) {
  return new FileStream(path.name.c_str(), flag);
}
ISeekStream *LocalFileSystem::OpenPartForRead(const URI &path, size_t begin_bytes) {
  FileStream *fp = new FileStream(path.name.c_str(), "r");
  fp->Seek(begin_bytes);
  return fp;
}
}  // namespace io
}  // namespace dmlc
