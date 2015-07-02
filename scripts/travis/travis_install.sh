#!/bin/bash
# Install dependencies of dmlc-core for travis CI, need sudo
apt-get -y update

if [ ${TASK} == "lint" ]; then
    apt-get install python-pip
    pip install cpplint pylint
else
    apt-get install \
        wget git curl libcurl4-openssl-dev
fi
