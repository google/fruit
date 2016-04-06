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


docker exec add-apt-repository -y ppa:ubuntu-toolchain-r/test
docker exec wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -

docker exec add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-${REPO_NAME} main"
docker exec add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-${REPO_NAME}-3.6 main"
docker exec add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-${REPO_NAME}-3.7 main"
docker exec add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-${REPO_NAME}-3.8 main"

docker exec apt-get update -qq
