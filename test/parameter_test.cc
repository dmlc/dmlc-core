#include <dmlc/parameter.h>

// this is actual pice of code
struct Param : public dmlc::Parameter<Param> {
  float learning_rate;
  int num_hidden;
  std::string name;
  // declare parameters in header file
  DMLC_DECLARE_PARAMETER(Param) {
    DMLC_DECLARE(num_hidden).set_range(0, 1000);
    DMLC_DECLARE(learning_rate).set_default(0.01f);
    DMLC_DECLARE(name).set_default("hello");
  }
};
// register it in cc file
DMLC_REGISTER_PARAMETER(Param);

int main(int argc, char *argv[]) {
  Param param;
  std::map<std::string, std::string> kwargs;
  for (int i = 0; i < argc; ++i) {
    char name[256], val[256];
    if (sscanf(argv[i], "%[^=]=%[^\n]", name, val) == 2) {
      printf("call set %s=%s\n", name, val);
      kwargs[name] = val;
    }
  }
  param.Init(kwargs);
  printf("-----\n");
  printf("param.num_hidden=%d\n", param.num_hidden);
  printf("param.learning_rate=%f\n", param.learning_rate);
  printf("param.name=%s\n", param.name.c_str());
  printf("param.size=%lu\n", sizeof(param));
  return 0;
}
