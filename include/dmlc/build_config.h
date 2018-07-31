#define FOPEN_64_PRESENT

#if !defined(FOPEN_64_PRESENT) && DMLC_USE_FOPEN64
  #warning "Redefining fopen64 with std::fopen"
  #define fopen64 std::fopen
#endif
