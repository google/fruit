#!/bin/bash

set -e

apt-get update -qq
apt-get install -y --no-install-recommends wget

wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | apt-key add -

# 1E9377A2BA9EF27F is the key for the ubuntu-toolchain-r PPA.
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1E9377A2BA9EF27F

apt-get update -qq
apt-get install -y --no-install-recommends \
    file \
    valgrind \
    make \
    cmake \
    libboost-dev \
    g++-4.8 \
    g++-4.9 \
    g++-5 \
    clang-3.5 \
    clang-3.6 \
    clang-3.7 \
    clang-3.8 \
    clang-3.9 \
    libc++-dev \
    python3-nose2 \
    python3-pip
