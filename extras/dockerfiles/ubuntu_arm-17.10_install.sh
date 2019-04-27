#!/bin/bash

set -e

#apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1E9377A2BA9EF27F

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-7 \
    g++-5 \
    clang-3.9 \
    clang-4.0 \
    clang-5.0 \
    python \
    python3-sh \
    python3-typed-ast \
    clang-format

pip3 install typed_ast
