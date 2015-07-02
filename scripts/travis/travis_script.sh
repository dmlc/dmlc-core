#!/bin/bash
# Main script of travis 
MAKE= make -j4

if [ ${TASK} -eq "lint" ]; then
    ${MAKE} lint
    exit 0
fi

if [ ${TASK} -eq "build" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    ${MAKE} all
    exit 0
fi

if [ ${TASK} -eq "build_test" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    echo "BUILD_TEST=1" >> config.mk
    ${MAKE} all
    exit 0
fi
