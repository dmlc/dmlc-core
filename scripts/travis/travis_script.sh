#!/bin/bash
# Main script of travis

if [ ${TASK} == "lint" ]; then
    make lint
fi

if [ ${TASK} == "build" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    echo "export CXX="${CXX} >> config.mk
    make all
fi

if [ ${TASK} == "build_test" ]; then
    cp make/config.mk .
    echo "USE_S3=1" >> config.mk
    echo "BUILD_TEST=1" >> config.mk
    echo "export CXX="${CXX} >> config.mk
    make all
fi
