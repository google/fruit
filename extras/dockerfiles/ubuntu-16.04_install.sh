#!/bin/bash

set -e

apt-get install -y --no-install-recommends \
    curl

# For the Bazel repository
curl https://bazel.build/bazel-release.pub.gpg | apt-key add -

apt-get install -y --allow-unauthenticated --no-install-recommends \
    clang-3.5 \
    clang-3.6 \
    clang-3.7 \
    clang-3.8 \
    clang-3.9 \
    clang-4.0 \
    g++-5 \
    g++-4.9 \
    g++-6 \
    python \
    bazel \
    git \
    openjdk-8-jdk \
    clang-format

pip3 install typed_ast
