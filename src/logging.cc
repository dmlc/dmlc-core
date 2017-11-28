// Copyright by Contributors
#include <dmlc/base.h>
#include <dmlc/logging.h>
#if DMLC_LOG_STACK_TRACE
#include <cxxabi.h>
#endif
using namespace std;

namespace dmlc {
#if DMLC_LOG_STACK_TRACE
/// Demangle the first C++ symbol found so stack traces are more legible, mangled C++ start with _Z
std::string demangle(char const* msg_str) {
  using namespace std;
  string msg(msg_str);
  size_t symbol_start = string::npos;
  size_t symbol_end = string::npos;
  if ( ((symbol_start = msg.find("_Z")) != string::npos)
       && (symbol_end = msg.find_first_of(" ", symbol_start)) ) {
    string left_of_symbol(msg, 0, symbol_start);
    string symbol(msg, symbol_start, symbol_end - symbol_start);
    string right_of_symbol(msg, symbol_end);

    int status = 0;
    size_t length = string::npos;
    char* demangled_symbol = abi::__cxa_demangle(symbol.c_str(), 0, &length, &status);
    if (demangled_symbol && status == 0 && length > 0) {
      string symbol_str(demangled_symbol, length - 1);
      free(demangled_symbol);
      ostringstream os;
      os << left_of_symbol << symbol_str << right_of_symbol;
      return os.str();
    }
  }
  return string(msg_str);
}


#endif
} // namespace dmlc
