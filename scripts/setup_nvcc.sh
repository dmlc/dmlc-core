#!/bin/bash
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
# Install nvcc and setup environment variable
set -e
if [ $# -lt 1 ]; then
    echo "Usage: <NVCC_PREFIX>"
fi

prefix=$1

# list of debs to download from nvidia

files=( \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-core-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-cublas-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-cublas-dev-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-cudart-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-cudart-dev-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-curand-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-curand-dev-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-nvrtc-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-nvrtc-dev-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-misc-headers-7-5_7.5-18_amd64.deb" \
  "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/libcuda1-352_352.93-0ubuntu1_amd64.deb" \
)

for item in ${files[*]}
do
    wget ${item}
    name=$(echo ${item} | tr "/" "\n" | tail -1)
    dpkg -x ${name} ${prefix}
done

