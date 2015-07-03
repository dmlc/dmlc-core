#!/bin/bash
# Main script of travis

# setup the env variables
export CPLUS_INCLUDE_PATH=${CPLUS_INCLUDE_PATH}:${CACHE_PREFIX}/include
export C_INCLUDE_PATH=${C_INCLUDE_PATH}:${CACHE_PREFIX}/include
export LIBRARY_PATH=${LIBRARY_PATH}:${CACHE_PREFIX}/lib
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${CACHE_PREFIX}/lib

alias make="make -j4"

if [ ${TASK} == "lint" ]; then
    make lint || exit -1
fi

if [ ${TASK} == "doc" ]; then
    make doc 2>log.txt
    (cat log.txt|grep warning) && exit -1
fi

if [ ${TASK} == "build" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    echo "export CXX="${CXX} >> config.mk
    make all || exit -1
fi

if [ ${TASK} == "unittest_gtest" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    echo "BUILD_TEST=1" >> config.mk
    echo "export CXX="${CXX} >> config.mk
    echo "export DMLC_LDFLAGS= -L"${CACHE_PREFIX}/lib >> config.mk
    make all || exit -1
    test/unittest/dmlc_unittest || exit -1
fi
