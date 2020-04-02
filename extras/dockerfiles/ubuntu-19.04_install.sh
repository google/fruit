#!/bin/bash

set -e

cat <<EOF >/etc/apt/sources.list.d/ubuntu-19.10_custom.list
deb http://apt.llvm.org/disco/ llvm-toolchain-disco main
deb-src http://apt.llvm.org/disco/ llvm-toolchain-disco main
# 9
deb http://apt.llvm.org/disco/ llvm-toolchain-disco-9 main
deb-src http://apt.llvm.org/disco/ llvm-toolchain-disco-9 main
# 10
deb http://apt.llvm.org/disco/ llvm-toolchain-disco-10 main
deb-src http://apt.llvm.org/disco/ llvm-toolchain-disco-10 main
EOF

apt-get update

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-7 \
    g++-8 \
    g++-9 \
    clang-6.0 \
    clang-7 \
    clang-8 \
    python \
    python3-sh \
    python3-typed-ast \
    clang-tidy \
    clang-format
