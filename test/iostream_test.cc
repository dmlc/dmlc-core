#include <iostream>
#include <dmlc/io.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: <filename>\n");
    return 0;
  }
  {// output
    dmlc::IStream *fs = dmlc::IStream::Create(argv[1], "w");
    dmlc::ostream os(fs);
    os << "hello-world " << 1e-10<< std::endl;
    delete fs;
  }
  {// input
    std::string name;
    double data;
    dmlc::IStream *fs = dmlc::IStream::Create(argv[1], "r");
    dmlc::istream is(fs);
    is >> name >> data;
    std::cout << name << " " << data << std::endl;
    delete fs;
  }
  return 0;
}
