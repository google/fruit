#!/bin/bash

set -e

apt-get update -qq
apt-get install -y --no-install-recommends wget gnupg ca-certificates apt-transport-https

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
    libc++1 \
    libc++-dev \
    libc++abi1 \
    libc++abi-dev \
    dirmngr \
    python \
    python3 \
    python3-setuptools
