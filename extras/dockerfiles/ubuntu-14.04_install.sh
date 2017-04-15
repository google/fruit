#!/bin/bash

set -e

apt-get install -y --allow-unauthenticated --no-install-recommends \
    clang-3.5 \
    clang-3.6

# python3-sh doesn't exist in Ubuntu 14.04, we must install it via pip instead.
pip3 install --user sh
