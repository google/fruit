#!/bin/bash

set -e

apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1E9377A2BA9EF27F

apt-get install -y --allow-unauthenticated --no-install-recommends \
    clang-3.7 \
    clang-3.8 \
    clang-3.9 \
    clang-4.0 \
    g++-5 \
    g++-4.9 \
    g++-6 \
    python \
    clang-format

pip3 install typed_ast
