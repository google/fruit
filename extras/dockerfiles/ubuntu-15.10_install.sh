#!/bin/bash

set -e

# For Bazel
apt-get install -y --no-install-recommends \
    openjdk-8-jdk \
    pkg-config \
    zip \
    g++ \
    zlib1g-dev \
    unzip
# TODO: keep this version (reasonably) up to date.
BAZEL_VERSION=0.2.2b
wget https://github.com/bazelbuild/bazel/releases/download/${BAZEL_VERSION}/bazel-${BAZEL_VERSION}-installer-linux-x86_64.sh
bash bazel-${BAZEL_VERSION}-installer-linux-x86_64.sh
rm bazel-${BAZEL_VERSION}-installer-linux-x86_64.sh

# python3-sh doesn't exist in Ubuntu 15.10, we must install it via pip instead.
pip3 install --user sh
