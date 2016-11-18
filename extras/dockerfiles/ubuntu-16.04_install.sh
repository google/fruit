#!/bin/bash

set -e

apt-get install -y --no-install-recommends \
    curl

# For the Bazel repository
curl https://bazel.build/bazel-release.pub.gpg | apt-key add -

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-6 \
    python \
    python3-sh \
    bazel
