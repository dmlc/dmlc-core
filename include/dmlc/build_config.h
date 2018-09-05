/* default detection logic for fopen64; may be overriden by CMake */

#if DMLC_USE_FOPEN64 && \
  (!defined(__GNUC__) || (defined __ANDROID__) || (defined __FreeBSD__) \
   || (defined __APPLE__) || ((defined __MINGW32__) && !(defined __MINGW64__)))
  #warning "Redefining fopen64 with std::fopen"
  #define fopen64 std::fopen
#endif
