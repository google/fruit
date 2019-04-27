#!/bin/bash

set -e

apt-get install -y --allow-unauthenticated --no-install-recommends \
    clang-3.5 \
    clang-3.6 \
    clang-3.7 \
    clang-3.9 \
    clang-4.0 \
    g++-5 \
    g++-4.9 \
    g++-6 \
    python \
    clang-format
