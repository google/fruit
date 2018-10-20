#!/bin/bash

set -e

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-7 \
    g++-8 \
    clang-6.0 \
    clang-7 \
    python \
    python3-sh \
    python3-typed-ast \
    clang-format
