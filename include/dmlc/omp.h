/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 *  Copyright (c) 2015 by Contributors
 * \file omp.h
 * \brief header to handle OpenMP compatibility issues
 */
#ifndef DMLC_OMP_H_
#define DMLC_OMP_H_
#if defined(_OPENMP)
#include <omp.h>
#else
#ifndef DISABLE_OPENMP
// use pragma message instead of warning
#pragma message("Warning: OpenMP is not available, "                    \
                "project will be compiled into single-thread code. "     \
                "Use OpenMP-enabled compiler to get benefit of multi-threading.")
#endif
//! \cond Doxygen_Suppress
#ifdef __cplusplus
extern "C" {
# define __GOMP_NOTHROW throw ()
#else
# define __GOMP_NOTHROW __attribute__((__nothrow__))
#endif
inline int omp_get_thread_num() __GOMP_NOTHROW { return 0; }
inline int omp_get_num_threads() __GOMP_NOTHROW { return 1; }
inline int omp_get_max_threads() __GOMP_NOTHROW { return 1; }
inline int omp_get_num_procs() __GOMP_NOTHROW { return 1; }
inline void omp_set_num_threads(int nthread) __GOMP_NOTHROW {}
#ifdef __cplusplus
}
#endif
#endif
// loop variable used in openmp
namespace dmlc {
#ifdef _MSC_VER
typedef int omp_uint;
typedef long omp_ulong;  // NOLINT(*)
#else
typedef unsigned omp_uint;
typedef unsigned long omp_ulong; // NOLINT(*)
#endif
//! \endcond
}  // namespace dmlc
#endif  // DMLC_OMP_H_
