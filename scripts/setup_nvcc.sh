# Install nvcc and setup environment variable
# Do not directly call it, source it in your script
# need to first set NVCC_PREFIX
set -e
if [ -z "$NVCC_PREFIX" ]; then
    echo "NVCC_PREFIX need to be set"
    exit -1
fi

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
    dpkg -x ${item} ${prefix}
done

export PATH=${PATH}:${prefix}/usr/local/cuda-7.0/bin
export CPLUS_INCLUDE_PATH=${CPLUS_INCLUDE_PATH}:${prefix}/usr/local/cuda-7.0/cuda-7.0/include
export C_INCLUDE_PATH=${C_INCLUDE_PATH}:${prefix}/usr/local/cuda-7.0/cuda-7.0/include
export LIBRARY_PATH=${LIBRARY_PATH}:${prefix}/usr/local/cuda-7.0/cuda-7.0/lib64
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${prefix}/usr/local/cuda-7.0/cuda-7.0/lib64
