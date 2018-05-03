#!/bin/bash
set -e
set -x

if [ ${TRAVIS_OS_NAME} != "osx" ]; then
    exit 0
fi

brew update
brew upgrade python3
python3 -m pip install --upgrade pip
