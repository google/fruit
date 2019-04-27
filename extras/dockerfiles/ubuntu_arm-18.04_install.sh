#!/bin/bash

set -e

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-8 \
    g++-7 \
    g++-5 \
    clang-3.9 \
    clang-4.0 \
    clang-5.0 \
    clang-6.0 \
    python \
    python3-sh \
    python3-typed-ast \
    clang-format
