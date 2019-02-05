#!/bin/bash
set -e
set -x

if [ ${TRAVIS_OS_NAME} != "osx" ]; then
    exit 0
fi

python3 -m pip install --upgrade pip
