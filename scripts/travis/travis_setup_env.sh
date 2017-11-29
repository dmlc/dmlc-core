# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
# script to be sourced in travis yml
# setup all enviroment variables

export CACHE_PREFIX=${HOME}/.cache/usr
export PATH=${HOME}/.local/bin:${PATH}
export PATH=${PATH}:${CACHE_PREFIX}/bin
export CPLUS_INCLUDE_PATH=${CPLUS_INCLUDE_PATH}:${CACHE_PREFIX}/include
export C_INCLUDE_PATH=${C_INCLUDE_PATH}:${CACHE_PREFIX}/include
export LIBRARY_PATH=${LIBRARY_PATH}:${CACHE_PREFIX}/lib
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CACHE_PREFIX}/lib
export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${CACHE_PREFIX}/lib

alias make="make -j4"

# setup the cache prefix folder
if [ ! -d ${HOME}/.cache ]; then
    mkdir ${HOME}/.cache
fi

if [ ! -d ${CACHE_PREFIX} ]; then
    mkdir ${CACHE_PREFIX}
fi
if [ ! -d ${CACHE_PREFIX}/include ]; then
    mkdir ${CACHE_PREFIX}/include
fi
if [ ! -d ${CACHE_PREFIX}/lib ]; then
    mkdir ${CACHE_PREFIX}/lib
fi
if [ ! -d ${CACHE_PREFIX}/bin ]; then
    mkdir ${CACHE_PREFIX}/bin
fi

# setup CUDA path if NVCC_PREFIX exists
if [ ! -z "$NVCC_PREFIX" ]; then
    export PATH=${PATH}:${NVCC_PREFIX}/usr/local/cuda-7.5/bin
    export CPLUS_INCLUDE_PATH=${CPLUS_INCLUDE_PATH}:${NVCC_PREFIX}/usr/local/cuda-7.5/include
    export C_INCLUDE_PATH=${C_INCLUDE_PATH}:${NVCC_PREFIX}/usr/local/cuda-7.5/include
    export LIBRARY_PATH=${LIBRARY_PATH}:${NVCC_PREFIX}/usr/local/cuda-7.5/lib64:${NVCC_PREFIX}/usr/lib/x86_64-linux-gnu
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${NVCC_PREFIX}/usr/local/cuda-7.5/lib64:${NVCC_PREFIX}/usr/lib/x86_64-linux-gnu
fi
