#!/bin/bash
# Install dependencies of dmlc-core for travis CI, need sudo
apt-get -y update
apt-get install \
    wget git curl libcurl4-openssl-dev\
    python-numpy python-pip
    
pip install cpplint pylint
