/*!
 * Copyright (c) 2018 by Contributors
 * \file build_config.h
 * \brief Default detection logic for fopen64 and other symbols.
 *        May be overriden by CMake
 * \author KOLANICH
 */
#ifndef DMLC_BUILD_CONFIG_H_
#define DMLC_BUILD_CONFIG_H_

#if DMLC_USE_FOPEN64 && \
  (!defined(__GNUC__) || (defined __ANDROID__) || (defined __FreeBSD__) \
  || (defined __APPLE__) || ((defined __MINGW32__) && !(defined __MINGW64__)))
  #define DMLC_EMIT_FOPEN64_REDEFINE_WARNING
  #define fopen64 std::fopen
#endif

/* default logic for locale functionality */
#ifdef _WIN32

  #define CREATE_LOCALE_PRESENT
  #define STRTOD_L_PRESENT
  #define FREE_LOCALE_PRESENT

#else  // _WIN32

  #define USE_LOCALE_PRESENT
  #define NEW_LOCALE_PRESENT
  #define FREE_LOCALE_PRESENT

#endif  // _WIN32

#endif  // DMLC_BUILD_CONFIG_H_
