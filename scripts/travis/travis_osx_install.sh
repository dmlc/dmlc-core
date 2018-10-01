#!/bin/bash
set -e
set -x

if [ ${TRAVIS_OS_NAME} != "osx" ]; then
    exit 0
fi

# Update command line tool to avoid an error:
# "_stdio.h: No such file or directory"
softwareupdate --list
softwareupdate --install "Command Line Tools (macOS High Sierra version 10.13) for Xcode-9.4"

brew update
brew upgrade python3
brew install gcc@7 || brew link --overwrite gcc@7
python3 -m pip install --upgrade pip
