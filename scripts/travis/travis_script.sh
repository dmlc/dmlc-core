#!/bin/bash

# main script of travis
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
    echo "USE_BLAS=blas" >> config.mk
    echo "USE_CUDA=0" >> config.mk
    echo "USE_CUDNN=0" >> config.mk
    echo "export CXX="${CXX} >> config.mk
    make all || exit -1
fi

if [ ${TASK} == "unittest_gtest" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    echo "BUILD_TEST=1" >> config.mk
    echo "export CXX="${CXX} >> config.mk
    make all || exit -1
    test/unittest/dmlc_unittest || exit -1
fi
