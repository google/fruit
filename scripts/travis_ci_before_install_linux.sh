#!/bin/bash -x

set -e

sudo apt-get update -qq
sudo apt-get install -qq --force-yes docker


docker pull ubuntu:$UBUNTU
docker run -d -it --name fruit --privileged ubuntu:$UBUNTU
docker exec fruit mkdir fruit
docker cp . fruit:/

case "$UBUNTU" in
14.04)
    REPO_NAME="trusty";;
15.10)
    REPO_NAME="xenial";;
*)
    echo "Unrecognized Ubuntu version: $UBUNTU"
    exit 1
esac

docker attach fruit <<EOF
set -e

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -

sudo add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-${REPO_NAME} main"
sudo add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-${REPO_NAME}-3.6 main"
sudo add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-${REPO_NAME}-3.7 main"
sudo add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-${REPO_NAME}-3.8 main"

sudo apt-get update -qq
EOF
