#!/bin/bash
# Main script of travis

if [ ${TASK} == "lint" ]; then
    make lint || exit -1
fi

if [ ${TASK} == "build" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    make all || exit -1
fi

if [ ${TASK} == "build_test" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    echo "BUILD_TEST=1" >> config.mk
    make all || exit -1
fi
