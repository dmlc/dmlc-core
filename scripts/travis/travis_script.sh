#!/bin/bash
# Main script of travis 
export MAKECMD= make -j4

if [ ${TASK} == "lint" ]; then
    ${MAKECMD} lint
fi

if [ ${TASK} == "build" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    ${MAKECMD} all
fi

if [ ${TASK} == "build_test" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    echo "BUILD_TEST=1" >> config.mk
    ${MAKECMD} all
fi
