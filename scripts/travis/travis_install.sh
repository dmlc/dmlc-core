#!/bin/bash
# Install dependencies of dmlc-core for travis CI

# Do not use sudo, to enable cache
pip install  cpplint pylint --user `whoami`

# setup thecache prefix, install all packages into here
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

# gtest
if [ ! -d ${CACHE_PREFIX}/include/gtest ]; then
    rm -rf gtest-1.7.0.zip gtest-1.7.0
    wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
    unzip gtest-1.7.0.zip
    cd gtest-1.7.0
    ./configure
    make
    cp -r include/gtest ${CACHE_PREFIX}/include
    cp -r lib/.libs/* ${CACHE_PREFIX}/lib
    cd ..
    rm -rf gtest-1.7.0.zip
fi
