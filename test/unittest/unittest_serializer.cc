#include <dmlc/io.h>
#include <dmlc/logging.h>
#include <gtest/gtest.h>
#include <sstream>
#include <cstring>
#include <unordered_map>
#include <iostream>

using namespace std;

/*! \brief a in memory buffer that can be read and write as stream interface */
struct MemoryBufferStream : public dmlc::SeekStream {
 public:
  explicit MemoryBufferStream(std::string *p_buffer)
      : p_buffer_(p_buffer) {
    curr_ptr_ = 0;
  }
  virtual ~MemoryBufferStream(void) {}
  virtual size_t Read(void *ptr, size_t size) {
    CHECK(curr_ptr_ <= p_buffer_->length())
        << "read can not have position excceed buffer length";
    size_t nread = std::min(p_buffer_->length() - curr_ptr_, size);
    if (nread != 0) std::memcpy(ptr, &(*p_buffer_)[0] + curr_ptr_, nread);
    curr_ptr_ += nread;
    return nread;
  }
  virtual void Write(const void *ptr, size_t size) {
    if (size == 0) return;
    if (curr_ptr_ + size > p_buffer_->length()) {
      p_buffer_->resize(curr_ptr_+size);
    }
    std::memcpy(&(*p_buffer_)[0] + curr_ptr_, ptr, size);
    curr_ptr_ += size;
  }
  virtual void Seek(size_t pos) {
    curr_ptr_ = static_cast<size_t>(pos);
  }
  virtual size_t Tell(void) {
    return curr_ptr_;
  }
  virtual bool AtEnd(void) const {
    return curr_ptr_ == p_buffer_->length();
  }

 private:
  /*! \brief in memory buffer */
  std::string *p_buffer_;
  /*! \brief current pointer */
  size_t curr_ptr_;
};  // class MemoryBufferStream

template<typename T>
inline void TestSaveLoad(T data) {
  std::string blob;
  MemoryBufferStream fs(&blob);
  {
    T temp(data);
    static_cast<dmlc::Stream*>(&fs)->Write(temp);
    temp.clear();
  }
  fs.Seek(0);
  T copy_data;
  CHECK(static_cast<dmlc::Stream*>(&fs)->Read(&copy_data));
  ASSERT_EQ(data, copy_data);
}

class MyClass {
 public:
  MyClass() {}
  MyClass(std::string data) : data_(data) {}
  inline void Save(dmlc::Stream *strm) const {
    strm->Write(this->data_);
  }
  inline bool Load(dmlc::Stream *strm) {
    return strm->Read(&data_);
  }
  inline bool operator==(const MyClass &other) const {
    return data_ == other.data_;
  }

 private:
  std::string data_;
};
// need to declare the traits property of my class to dmlc
namespace dmlc { DMLC_DECLARE_TRAITS(has_saveload, MyClass, true); }

struct Param {
  int a;
  int b;
  Param() {}
  Param(int a, int b) : a(a), b(b) {}
  inline bool operator==(const Param &other) const {
    return a == other.a && b == other.b;
  }
};
// need to declare the traits property of my class to dmlc
namespace dmlc { DMLC_DECLARE_TRAITS(is_pod, Param, true); }

// test serializer
TEST(Serializer, basics) {
  int n = 10;
  std::vector<int> a;
  for (int i = 0; i < n; ++i) {
    a.push_back(i);
  }
  TestSaveLoad(a);

  std::vector<std::string> b;
  for (int i = 0; i < n; ++i) {
    std::string ss(i, 'a' + (i % 26));
    b.push_back(ss);
  }
  TestSaveLoad(b);

  std::vector<std::vector<int> > temp {{1,2,3}, {1,2}, {1,2,3,4}};
  TestSaveLoad(temp);
  TestSaveLoad(
      std::map<int, std::string>  {{1, "hellkow"}, {2, "world"}});
  TestSaveLoad(
      std::unordered_map<int, std::string>  {{1, "hellkow"}, {2, "world"}});
  TestSaveLoad(
      std::unordered_multimap<int, std::string>  {{1, "hellkow"}, {1, "world"}, {2, "111"}});
  TestSaveLoad(std::set<std::string>  {"hjhjm", "asasa"});
  TestSaveLoad(std::unordered_set<std::string>  {"hjhjm", "asasa"});
  TestSaveLoad(std::list<std::string>  {"hjhjm", "asasa"});
  TestSaveLoad(std::list<int>(a.begin(), a.end()));
  TestSaveLoad(std::list<MyClass> {MyClass("abc"), MyClass("def")});
  TestSaveLoad(std::list<Param> {Param(3, 4), Param(5, 6)});

}
