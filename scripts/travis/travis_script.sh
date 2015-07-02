#!/bin/bash
# Main script of travis 
export MAKECMD= make -j4

if [ ${TASK} == "lint" ]; then
    ${MAKECMD} lint
    exit 0
fi

if [ ${TASK} == "build" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    ${MAKECMD} all
    exit 0
fi

if [ ${TASK} == "build_test" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    echo "BUILD_TEST=1" >> config.mk
    ${MAKECMD} all
    exit 0
fi
