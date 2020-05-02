#!/bin/bash

set -e

cat <<EOF >/etc/apt/sources.list.d/ubuntu-20.04_custom.list
deb http://apt.llvm.org/focal/ llvm-toolchain-focal-9 main
deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal-9 main
deb http://apt.llvm.org/focal/ llvm-toolchain-focal-10 main
deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal-10 main
EOF

apt-get update

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-7 \
    g++-8 \
    g++-9 \
    g++-10 \
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
