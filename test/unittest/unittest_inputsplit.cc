#include <dmlc/data.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <future>
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
    LOG(INFO) << "Created temporary directory " << path;
  }

  ~TemporaryDirectory() {
    for (const std::string& filename : file_list) {
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
      LOG(INFO) << "Successfully deleted temporary directory " << path;
    } else {
      LOG(FATAL) << "~TemporaryDirectory(): "
                 << "Could not remove temporary directory " << path;
    }
  }

  std::string AddFile(const std::string& filename) {
    const std::string file_path = this->path + "/" + filename;
    file_list.push_back(file_path);
    return file_path;
  }

  std::string path;

 private:
  std::vector<std::string> file_list;
};

TEST(InputSplit, test_split_csv_noeol) {
  size_t num_row, num_col;
  {
    /* Create a test case for partitioned csv with NOEOL */
    TemporaryDirectory tempdir;
    {
      const std::string filename = tempdir.AddFile("train_0.csv");
      std::ofstream of(filename.c_str(), std::ios::binary);
      of << "0,1,1,1";  // NOEOL (no '\n' at end of file)
    }
    {
      const std::string filename = tempdir.AddFile("train_1.csv");
      std::ofstream of(filename.c_str(), std::ios::binary);
      of << "0,1,1,2\n";
    }
    {
      const std::string filename = tempdir.AddFile("train_2.csv");
      std::ofstream of(filename.c_str(), std::ios::binary);
      of << "0,1,1,2\n";
    }
    /* Load the test case with InputSplit and obtain matrix dimensions */
    {
      std::unique_ptr<dmlc::Parser<uint32_t> > parser(
        dmlc::Parser<uint32_t>::Create(tempdir.path.c_str(), 0, 1, "csv"));
      CountDimensions(parser.get(), &num_row, &num_col);
    }
  }
  /* Check matrix dimensions: must be 3x4 */
  ASSERT_EQ(num_row, 3U);
  ASSERT_EQ(num_col, 4U);
}

TEST(InputSplit, test_split_libsvm_noeol) {
  {
    /* Create a test case for partitioned libsvm with NOEOL */
    TemporaryDirectory tempdir;
    const char* line
      = "1 3:1 10:1 11:1 21:1 30:1 34:1 36:1 40:1 41:1 53:1 58:1 65:1 69:1 "
        "77:1 86:1 88:1 92:1 95:1 102:1 105:1 117:1 124:1";
    {
      const std::string filename = tempdir.AddFile("train_0.libsvm");
      std::ofstream of(filename.c_str(), std::ios::binary);
      of << line << "\n";
    }
    {
      const std::string filename = tempdir.AddFile("train_1.libsvm");
      std::ofstream of(filename.c_str(), std::ios::binary);
      of << line;  // NOEOL (no '\n' at end of file)
    }
    /* Run test with 1 sec timeout */
    std::promise<bool> finish_flag;
    auto finish_flag_result = finish_flag.get_future();
    std::thread([&tempdir](std::promise<bool>& finish_flag) {
      std::unique_ptr<dmlc::Parser<uint32_t> > parser(
        dmlc::Parser<uint32_t>::Create(tempdir.path.c_str(), 0, 1, "libsvm"));
      size_t num_row, num_col;
      CountDimensions(parser.get(), &num_row, &num_col);
      finish_flag.set_value(true);
    }, std::ref(finish_flag)).detach();
    EXPECT_TRUE(finish_flag_result.wait_for(std::chrono::milliseconds(1000))
                != std::future_status::timeout);
  }
}

TEST(InputSplit, test_split_libsvm) {
  size_t num_row, num_col;
  {
    /* Create a test case for partitioned libsvm */
    TemporaryDirectory tempdir;
    const int nfile = 5;
    for (int file_id = 0; file_id < nfile; ++file_id) {
      const std::string filename
        = tempdir.AddFile(std::string("test_") + std::to_string(file_id) + ".libsvm");
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
  }
  /* Check matrix dimensions: must be 5x125 */
  ASSERT_EQ(num_row, 5U);
  ASSERT_EQ(num_col, 125U);
}

TEST(InputSplit, test_split_libsvm_distributed) {
  {
    /* Create a test case for partitioned libsvm */
    TemporaryDirectory tempdir;
    const char* line
      = "1 3:1 10:1 11:1 21:1 30:1 34:1 36:1 40:1 41:1 53:1 58:1 65:1 69:1 "
        "77:1 86:1 88:1 92:1 95:1 102:1 105:1 117:1 124:1\n";
    const int nfile = 5;
    for (int file_id = 0; file_id < nfile; ++file_id) {
      const std::string filename
        = tempdir.AddFile(std::string("test_") + std::to_string(file_id) + ".libsvm");
      std::ofstream of(filename.c_str(), std::ios::binary);
      const int nrepeat = (file_id == 0 ? 6 : 1);
      for (int i = 0; i < nrepeat; ++i) {
        of << line;
      }
    }

    /* Load the test case with InputSplit and obtain matrix dimensions */
    const int npart = 2;
    const size_t expected_dims[npart][2] = { {6, 125}, {4, 125} };
    for (int part_id = 0; part_id < npart; ++part_id) {
      std::unique_ptr<dmlc::Parser<uint32_t> > parser(
        dmlc::Parser<uint32_t>::Create(tempdir.path.c_str(), part_id, npart, "libsvm"));
      size_t num_row, num_col;
      CountDimensions(parser.get(), &num_row, &num_col);
      ASSERT_EQ(num_row, expected_dims[part_id][0]);
      ASSERT_EQ(num_col, expected_dims[part_id][1]);
    }
  }
}
