#include "../src/data/csv_parser.h"
#include "../src/data/libsvm_parser.h"
#include "../include/dmlc/data.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <gtest/gtest.h>

/* platform specific headers */
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#else
#include <unistd.h>
#endif

static inline void CountDimensions(dmlc::Parser<uint32_t>* parser,
                                   size_t* out_num_row, size_t* out_num_col) {
  size_t num_row = 0;
  size_t num_col = 0;
  while (parser->Next()) {
    const dmlc::RowBlock<uint32_t>& batch = parser->Value();
    num_row += batch.size;
    for (size_t i = batch.offset[0]; i < batch.offset[batch.size]; ++i) {
      const uint32_t index = batch.index[i];
      num_col = std::max(num_col, static_cast<size_t>(index + 1));
    }
  }
  *out_num_row = num_row;
  *out_num_col = num_col;
}

class TemporaryDirectory {
 public:
  TemporaryDirectory() {
#if _WIN32
    /* locate the root directory of temporary area */
    char tmproot[MAX_PATH] = {0};
    const DWORD dw_retval = GetTempPathA(MAX_PATH, tmproot);
    if (dw_retval > MAX_PATH || dw_retval == 0) {
      std::cerr << "TemporaryDirectory(): "
                << "Could not create temporary directory" << std::endl;
      exit(-1);
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
      std::cerr << "TemporaryDirectory(): "
                << "Could not create temporary directory" << std::endl;
      exit(-1);
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
      std::cerr << "TemporaryDirectory(): "
                << "Could not create temporary directory" << std::endl;
      exit(-1);
    }
    path = std::string(tmpdir);
#endif
    std::cerr << "Created temporary directory " << path << std::endl;
  }
  ~TemporaryDirectory() {
#if _WIN32
    if (!RemoveDirectoryA(path.c_str())) {
#else
    if (rmdir(path.c_str()) == -1) {
#endif
      std::cerr << "~TemporaryDirectory(): "
                << "Could not remove temporary directory " << path << std::endl;
      exit(-1);
    }
    std::cerr << "Successfully deleted temporary directory " << path << std::endl;
  }
  std::string path;
};

TEST(InputSplit, test_split_csv_noeol) {
  size_t num_row, num_col;
  {
    /* Create a test case for partitioned csv with NOEOL */
    TemporaryDirectory tempdir;
    {
      std::string filename = tempdir.path + "/train_0.csv";
      std::ofstream of(filename.c_str(), std::ios::binary);
      of << "0,1,1,1";  // NOEOL (no '\n' at end of file)
    }
    {
      std::string filename = tempdir.path + "/train_1.csv";
      std::ofstream of(filename.c_str(), std::ios::binary);
      of << "0,1,1,2\n";
    }
    {
      std::string filename = tempdir.path + "/train_2.csv";
      std::ofstream of(filename.c_str(), std::ios::binary);
      of << "0,1,1,2\n";
    }
    /* Load the test case with InputSplit and obtain matrix dimensions */
    {
      std::unique_ptr<dmlc::Parser<uint32_t> > parser(
        dmlc::Parser<uint32_t>::Create(tempdir.path.c_str(), 0, 1, "csv"));
      CountDimensions(parser.get(), &num_row, &num_col);
    }
    /* Clean up */
    for (int i = 0; i < 3; ++i) {
      std::string filename
        = tempdir.path + "/train_" + std::to_string(i) + ".csv";
      if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Couldn't remove file " << filename << std::endl;
        exit(-1);
      }
    }
  }
  /* Check matrix dimensions: must be 3x4 */
  ASSERT_EQ(num_row, 3U);
  ASSERT_EQ(num_col, 4U);
}

TEST(InputSplit, test_split_libsvm) {
  size_t num_row, num_col;
  {
    /* Create a test case for partitioned libsvm */
    TemporaryDirectory tempdir;
    for (int i = 0; i < 5; ++i) {
      std::string filename
        = tempdir.path + "/test_" + std::to_string(i) + ".libsvm";
      std::ofstream of(filename.c_str(), std::ios::binary);
      of << "1 3:1 10:1 11:1 21:1 30:1 34:1 36:1 40:1 41:1 53:1 58:1 65:1 69:1 "
         << "77:1 86:1 88:1 92:1 95:1 102:1 105:1 117:1 124:1\n";
    }
    /* Load the test case with InputSplit and obtain matrix dimensions */
    {
      std::unique_ptr<dmlc::Parser<uint32_t> > parser(
        dmlc::Parser<uint32_t>::Create(tempdir.path.c_str(), 0, 1, "libsvm"));
      CountDimensions(parser.get(), &num_row, &num_col);
    }
    /* Clean up */
    for (int i = 0; i < 5; ++i) {
      std::string filename
        = tempdir.path + "/test_" + std::to_string(i) + ".libsvm";
      if (std::remove(filename.c_str()) != 0) {
        std::cerr << "Couldn't remove file " << filename << std::endl;
        exit(-1);
      }
    }
  }
  /* Check matrix dimensions: must be 5x125 */
  ASSERT_EQ(num_row, 5U);
  ASSERT_EQ(num_col, 125U);
}
