#!/bin/bash

set -e

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo add-apt-repository -y 'deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty main'
sudo add-apt-repository -y 'deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.6 main'
sudo add-apt-repository -y 'deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main'
sudo apt-get update -qq
