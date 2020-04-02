#!/bin/bash

set -e

cat <<EOF >/etc/apt/sources.list.d/ubuntu-19.10_custom.list
deb http://apt.llvm.org/eoan/ llvm-toolchain-eoan main
deb-src http://apt.llvm.org/eoan/ llvm-toolchain-eoan main
deb http://apt.llvm.org/eoan/ llvm-toolchain-eoan-9 main
deb-src http://apt.llvm.org/eoan/ llvm-toolchain-eoan-9 main
deb http://apt.llvm.org/eoan/ llvm-toolchain-eoan-10 main
deb-src http://apt.llvm.org/eoan/ llvm-toolchain-eoan-10 main
EOF

apt-get update

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-7 \
    g++-8 \
    g++-9 \
    clang-6.0 \
    clang-7 \
    clang-8 \
    clang-9 \
    clang-10 \
    python \
    python3-sh \
    python3-typed-ast \
    clang-tidy \
    clang-format
