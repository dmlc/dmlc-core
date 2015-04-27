#include <dmlc/io.h>
#include <dmlc/logging.h>
#include <chrono>
#include <dmlc/threadediter.h>

using namespace dmlc;

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
struct IntProducer : public ThreadedIter<int>::Producer {
  int counter;
  int maxcap;
  int sleep;
  IntProducer(int maxcap, int sleep)
      : counter(0), maxcap(maxcap), sleep(sleep) {}
  virtual void BeforeFirst(void) {
    counter = 0;
  }
  virtual bool Next(int **inout_dptr) {
    if (counter == maxcap) return false;    
    // allocate space if not exist
    if (*inout_dptr == NULL) {
      *inout_dptr = new int();
    }
    delay(sleep);
    **inout_dptr = counter++;
    return true;
  }
};

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Usage: maxcap delay-producer delay-consumer\n");
    return 0;
  }  
  ThreadedIter<int> iter;
  iter.set_max_capacity(10);
  IntProducer prod(atoi(argv[1]), atoi(argv[2]));
  int d = atoi(argv[3]);
  iter.Init(&prod);
  int counter = 0;
  while (iter.Next()) {
    CHECK(counter == iter.Value());
    delay(d);
    printf("%d\n", counter);
    ++counter;
  }
  CHECK(!iter.Next());
  iter.BeforeFirst();
  int *value;
  while (iter.Next(&value)) {
    printf("%d\n", *value);
    iter.Recycle(&value);
    delay(d);
    CHECK(value == NULL);
  }
  printf("finish\n");
  return 0;
}
