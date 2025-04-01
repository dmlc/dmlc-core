#!/bin/bash

set -e
set -x

# For all tests other than s390x_test, expect little endian
export DMLC_UNIT_TEST_LITTLE_ENDIAN=1

if [[ ${TASK} == "cmake_test" ]]; then
    # Build dmlc-core with CMake, including unit tests
    rm -rf build
    mkdir build && cd build
    cmake .. -GNinja -DGOOGLE_TEST=ON -DUSE_PARQUET=ON -DParquet_DIR=$CONDA_PREFIX/lib/cmake/arrow -DCMAKE_CXX_FLAGS="-Wall -Wextra -Werror"
    ninja -v
    ./test/dmlc_unit_tests
fi

if [[ ${TASK} == "sanitizer_test" ]]; then
    rm -rf build
    mkdir build && cd build
    cmake .. -GNinja -DGOOGLE_TEST=ON -DDMLC_USE_SANITIZER=ON -DUSE_PARQUET=ON \
             -DParquet_DIR=$CONDA_PREFIX/lib/cmake/arrow \
             -DDMLC_ENABLED_SANITIZERS="thread" -DCMAKE_BUILD_TYPE=Debug ..
    ninja
    ./test/unittest/dmlc_unit_tests || true   # For now just display sanitizer errors
    rm -rf *
    cmake .. -GNinja -DGOOGLE_TEST=ON -DDMLC_USE_SANITIZER=ON -DUSE_PARQUET=ON \
             -DParquet_DIR=$CONDA_PREFIX/lib/cmake/arrow \
             -DDMLC_ENABLED_SANITIZERS="leak;address" -DCMAKE_BUILD_TYPE=Debug ..
    ninja
    ./test/unittest/dmlc_unit_tests || true   # For now just display sanitizer errors
fi

if [[ ${TASK} == "s390x_test" ]]; then
    # Run unit tests inside emulated s390x Docker container (uses QEMU transparently).
    # This should help us achieve compatibility with big endian targets.
    scripts/s390x/build_via_cmake.sh
    DMLC_UNIT_TEST_LITTLE_ENDIAN=0 build/test/dmlc_unit_tests
fi
