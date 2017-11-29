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
 * \file timer.h
 * \brief cross platform timer for timing
 * \author Tianqi Chen
 */
#ifndef DMLC_TIMER_H_
#define DMLC_TIMER_H_

#include "base.h"

#if DMLC_USE_CXX11
#include <chrono>
#endif

#include <time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#include "./logging.h"

namespace dmlc {
/*!
 * \brief return time in seconds
 */
inline double GetTime(void) {
  #if DMLC_USE_CXX11
  return std::chrono::duration<double>(
      std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  #elif defined __MACH__
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  CHECK(clock_get_time(cclock, &mts) == 0) << "failed to get time";
  mach_port_deallocate(mach_task_self(), cclock);
  return static_cast<double>(mts.tv_sec) + static_cast<double>(mts.tv_nsec) * 1e-9;
  #else
  #if defined(__unix__) || defined(__linux__)
  timespec ts;
  CHECK(clock_gettime(CLOCK_REALTIME, &ts) == 0) << "failed to get time";
  return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) * 1e-9;
  #else
  return static_cast<double>(time(NULL));
  #endif
  #endif
}
}  // namespace dmlc
#endif  // DMLC_TIMER_H_
