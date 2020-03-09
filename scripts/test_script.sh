#!/bin/bash

set -e
set -x

if [[ ${TASK} == "lint" ]]; then
    # stop the build if there are Python syntax errors or undefined names
    python3 -m flake8 . --count --select=E901,E999,F821,F822,F823 --show-source --statistics
    # exit-zero treats all errors as warnings.  The GitHub editor is 127 chars wide
    python3 -m flake8 . --count --exit-zero --max-complexity=10 --max-line-length=127 --statistics

    make lint
    make doxygen 2>log.txt
    (cat log.txt| grep -v ENABLE_PREPROCESSING |grep -v "unsupported tag" |grep warning) && exit 1
    exit 0
fi

# For all tests other than s390x_test, expect little endian
export DMLC_UNIT_TEST_LITTLE_ENDIAN=1

if [[ ${TASK} == "unittest_gtest" ]]; then
    cp make/config.mk .
    if [[ $(uname) != "Darwin" ]]; then
        echo "USE_S3=1" >> config.mk
        echo "export CXX = g++-4.8" >> config.mk
        export CXX=g++-4.8
    else
        echo "USE_S3=0" >> config.mk
        echo "USE_OPENMP=1" >> config.mk
        echo "export CXX=g++-8" >> config.mk
        export CXX=g++-8
    fi
    make -f scripts/packages.mk gtest
    echo "GTEST_PATH="/tmp/gtest >> config.mk
    echo "BUILD_TEST=1" >> config.mk
    make all
    test/unittest/dmlc_unittest
fi

if [[ ${TASK} == "cmake_test" ]]; then
    # Build dmlc-core with CMake, including unit tests
    rm -rf build
    mkdir build && cd build
    cmake .. -DGOOGLE_TEST=ON
    make
    cd ..
    ./build/test/unittest/dmlc_unit_tests
fi

if [[ ${TASK} == "sanitizer_test" ]]; then
    rm -rf build
    mkdir build && cd build
    cmake .. -DGOOGLE_TEST=ON -DDMLC_USE_SANITIZER=ON \
             -DDMLC_ENABLED_SANITIZERS="thread" -DCMAKE_BUILD_TYPE=Debug ..
    make -j2
    cd ..
    ./build/test/unittest/dmlc_unit_tests || true   # For now just display sanitizer errors
fi

if [[ ${TASK} == "s390x_test" ]]; then
    # Run unit tests inside emulated s390x Docker container (uses QEMU transparently).
    # This should help us achieve compatibility with big endian targets.
    scripts/s390x/ci_build.sh s390_container scripts/s390x/build_via_cmake.sh
    scripts/s390x/ci_build.sh s390_container -e DMLC_UNIT_TEST_LITTLE_ENDIAN=0 build/test/unittest/dmlc_unit_tests
fi
