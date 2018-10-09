/*!
 * Copyright (c) 2018 by Contributors
 * \file build_config.cc
 * \brief Companion source for build_config.h
 * \author Hyunsu Philip Cho
 */

#include <dmlc/base.h>

#ifdef DMLC_EMIT_FOPEN64_REDEFINE_WARNING
  #warning "Redefining fopen64 with std::fopen"
#endif  // DMLC_EMIT_FOPEN64_REDEFINE_WARNING
