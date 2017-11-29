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
 *  Copyright (c) 2017 by Contributors
 * \file endian.h
 * \brief Endian testing
 */
#ifndef DMLC_ENDIAN_H_
#define DMLC_ENDIAN_H_

#if defined(__APPLE__) || defined(_WIN32)
#define DMLC_LITTLE_ENDIAN 1
#else
#include <endian.h>
#define DMLC_LITTLE_ENDIAN (__BYTE_ORDER == __LITTLE_ENDIAN)
#endif

#endif  // DMLC_ENDIAN_H_

