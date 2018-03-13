#include <chrono>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include <dmlc/threadediter.h>
#include <gtest/gtest.h>

using namespace dmlc;
namespace producer_test {
inline void delay(int sleep) {
  if (sleep < 0) {
    int d = rand() % (-sleep);
    std::this_thread::sleep_for(std::chrono::milliseconds(d));
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
  }
}

// int was only used as example, in real life
// use big data blob
struct IntProducerNextExc : public ThreadedIter<int>::Producer {
  int counter;
  int maxcap;
  int sleep;
  IntProducerNextExc(int maxcap, int sleep)
      : counter(0), maxcap(maxcap), sleep(sleep) {}
  virtual void BeforeFirst(void) { counter = 0; }
  virtual bool Next(int **inout_dptr) {
    if (counter == maxcap)
      return false;
    if (counter == (maxcap - 1)) {
      counter++;
      LOG(FATAL) << "Test Throw exception";
    }
    // allocate space if not exist
    if (*inout_dptr == NULL) {
      *inout_dptr = new int();
    }
    delay(sleep);
    **inout_dptr = counter++;
    return true;
  }
};

struct IntProducerBeforeFirst : public ThreadedIter<int>::Producer {
  IntProducerBeforeFirst() {}
  virtual void BeforeFirst(void) {
    LOG(FATAL) << "Throw exception in before first";
  }
  virtual bool Next(int **inout_dptr) { return true; }
};
}

TEST(ThreadedIter, exception) {
  using namespace producer_test;
  int *value;
  ThreadedIter<int> iter2;
  iter2.set_max_capacity(7);
  IntProducerNextExc prod(5, 100);
  bool caught = false;
  iter2.Init(&prod);
  iter2.BeforeFirst();
  try {
    delay(700);
    iter2.Recycle(&value);
  } catch (dmlc::Error &e) {
    caught = true;
    LOG(INFO) << "recycle exception caught";
  }
  CHECK(caught);
  iter2.Init(&prod);
  caught = false;
  iter2.BeforeFirst();
  try {
    while (iter2.Next(&value)) {
      iter2.Recycle(&value);
    }
  } catch (dmlc::Error &e) {
    caught = true;
    LOG(INFO) << "next exception caught";
  }
  CHECK(caught);
  LOG(INFO) << "finish";
  ThreadedIter<int> iter3;
  iter3.set_max_capacity(1);
  IntProducerBeforeFirst prod2;
  iter3.Init(&prod2);
  caught = false;
  try {
    iter3.BeforeFirst();
  } catch (dmlc::Error &e) {
    caught = true;
    LOG(INFO) << "beforefirst exception caught";
  }
  caught = false;
  try {
  iter3.BeforeFirst();
  } catch (dmlc::Error &e) {
    LOG(INFO) << "beforefirst exception thrown/caught";
    caught = true;
  }
  CHECK(caught);
}
