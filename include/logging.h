/*!
 *  Copyright (c) 2015 by Contributors
 * \file logging.h
 * \brief defines logging macros of dmlc
 *  allows use of GLOG, fall back to internal
 *  implementation when disabled
 */
#ifndef DMLC_LOGGING_H_
#define DMLC_LOGGING_H_
#include <cstdio>
#include <cstdlib>
#include <string>
#include "./base.h"

#ifndef DMLC_STRICT_CXX98_
#include <cstdarg>
#endif

#if DMLC_USE_GLOG
#include <glog/logging.h>
#else
// define simple logging command here
#endif
namespace dmlc {
#ifndef DMLC_CUSTOMIZE_ERROR_
/*! 
 * \brief handling of user error,
 *  caused by inappropriate input
 * \param msg error message 
 */
inline void HandleUserError(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(-1);
}
#else
void HandleCheckError(const char *msg);
#endif

#ifdef RABIT_STRICT_CXX98_
extern "C" void (*Error)(const char *fmt, ...);
#else
/*!
 * \brief report error message to user
 *   use this function instead of logger when
 *   the error was caused by bad input parameter
 *   provided by the user, the error handling
 *   could be redirected to customized handler
 * \param fmt format string
 */
inline void Error(const char *fmt, ...) {
  const int kPrintBuffer = 1 << 12;
  std::string msg(kPrintBuffer, '\0');
  va_list args;
  va_start(args, fmt);
  vsnprintf(&msg[0], kPrintBuffer, fmt, args);
  va_end(args);
  HandleUserError(msg.c_str());
}
#endif
}  // namespace dmlc
#endif  // DMLC_LOGGING_H_
