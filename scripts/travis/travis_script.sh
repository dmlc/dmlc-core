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

# main script of travis
if [ ${TASK} == "lint" ]; then
    make lint || exit -1
    make doxygen 2>log.txt
    (cat log.txt| grep -v ENABLE_PREPROCESSING |grep -v "unsupported tag" |grep warning) && exit -1
    exit 0
fi

if [ ${TRAVIS_OS_NAME} == "osx" ]; then
    export NO_OPENMP=1
fi

if [ ${TASK} == "unittest_gtest" ]; then
    cp make/config.mk .
    make -f scripts/packages.mk gtest
    if [ ${TRAVIS_OS_NAME} != "osx" ]; then
        echo "USE_S3=1" >> config.mk
        echo "export CXX = g++-4.8" >> config.mk
    else
        echo "USE_S3=0" >> config.mk
        echo "USE_OPENMP=0" >> config.mk
    fi
    echo "GTEST_PATH="${CACHE_PREFIX} >> config.mk
    echo "BUILD_TEST=1" >> config.mk
    make all || exit -1
    test/unittest/dmlc_unittest || exit -1
fi
