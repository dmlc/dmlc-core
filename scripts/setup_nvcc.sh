#!/bin/bash
# Install nvcc and setup environment variable
set -e
if [ $# -lt 1 ]; then
    echo "Usage: <NVCC_PREFIX>"
fi

prefix=$1

# list of debs to download from nvidia
files=( \
    "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-core-7-0_7.0-28_amd64.deb" \
    "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-cudart-dev-7-0_7.0-28_amd64.deb" \
    "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-cudart-7-0_7.0-28_amd64.deb" \
    "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-misc-headers-7-0_7.0-28_amd64.deb" \
    "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-cublas-7-0_7.0-28_amd64.deb" \
    "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-cublas-dev-7-0_7.0-28_amd64.deb" \
    "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-curand-7-0_7.0-28_amd64.deb" \
    "http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1404/x86_64/cuda-curand-dev-7-0_7.0-28_amd64.deb" \
)

for item in ${files[*]}
do
    wget ${item}
    name=$(echo ${item} | tr "/" "\n" | tail -1)
    dpkg -x ${name} ${prefix}
done

