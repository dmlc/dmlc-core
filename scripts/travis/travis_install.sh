#!/bin/bash
# Install dependencies of dmlc-core for travis CI
# Do not use sudo, to enable cache

# Build additional dependencies from source

pip install -r cpplint pylint --user `whoami`

if [ ! -d ${CACHE_PREFIX} ]; then
    mkdir ${CACHE_PREFIX}
    mkdir ${CACHE_PREFIX}/include
    mkdir ${CACHE_PREFIX}/lib
fi

# gtest
if [ ! -d ${CACHE_PREFIX}/include/gtest ]; then
    rm -rf gtest-1.7.0.zip gtest-1.7.0
    wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
    unzip gtest-1.7.0.zip
    cd gtest-1.7.0
    ./configure
    make
    cp -rf include/gtest ${CACHE_PREFIX}/include
    cp -rf lib/.libs/* ${CACHE_PREFIX}/lib
    cd ..
    rm -rf gtest-1.7.0.zip
fi
