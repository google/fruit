#!/bin/bash

set -e

apt-get update -qq
apt-get install -y --no-install-recommends wget gnupg

wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | apt-key add -

# 1E9377A2BA9EF27F is the key for the ubuntu-toolchain-r PPA.
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1E9377A2BA9EF27F | cat

# 15CF4D18AF4F7421 is the key for the http://apt.llvm.org/artful PPA.
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 15CF4D18AF4F7421 | cat

apt-get update -qq
apt-get install -y --allow-unauthenticated --no-install-recommends \
    file \
    valgrind \
    make \
    cmake \
    libboost-dev \
    g++-5 \
    clang-3.9 \
    clang-4.0 \
    libc++-dev \
    python3-pip \
    python3-setuptools \
    python3-networkx \
    dirmngr
    
pip3 install wheel
pip3 install pytest
pip3 install pytest-xdist
pip3 install sh
pip3 install bidict
