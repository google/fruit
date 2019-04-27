#!/bin/bash

set -e

apt-get install -y --no-install-recommends \
    curl

# For the Bazel repository
curl https://bazel.build/bazel-release.pub.gpg | apt-key add -

echo 'deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8' >> /etc/apt/sources.list.d/bazel.list

apt-get update -qq

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-8 \
    g++-7 \
    g++-5 \
    clang-3.9 \
    clang-4.0 \
    clang-5.0 \
    clang-6.0 \
    bazel \
    git \
    python \
    python3-sh \
    python3-typed-ast \
    clang-format
