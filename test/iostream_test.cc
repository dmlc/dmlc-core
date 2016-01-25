#include <iostream>
#include <dmlc/io.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: <filename>\n");
    return 0;
  }
  {// output
    dmlc::ostream os(argv[1]);
    os << "hello-world " << 1e-10<< std::endl;
  }
  {// input
    std::string name;
    double data;
    dmlc::istream is(argv[1]);
    is >> name >> data;
    std::cout << name << " " << data << std::endl;
  }
  return 0;
}
