/*!
 * Copyright (c) 2018 by Contributors
 * \file build_config.h
 * \brief Default detection logic for fopen64. May be overriden by CMake
 * \author KOLANICH
 */
#ifndef DMLC_BUILD_CONFIG_H_
#define DMLC_BUILD_CONFIG_H_

#if DMLC_USE_FOPEN64 && \
  (!defined(__GNUC__) || (defined __ANDROID__) || (defined __FreeBSD__) \
  || (defined __APPLE__) || ((defined __MINGW32__) && !(defined __MINGW64__)))
  #warning "Redefining fopen64 with std::fopen"
  #define fopen64 std::fopen
#endif

#endif  // DMLC_BUILD_CONFIG_H_
