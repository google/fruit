#!/bin/bash

set -e

cat <<EOF >/etc/apt/sources.list.d/custom.list
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial main
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.8 main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.8 main
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-4.0 main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-4.0 main
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-9 main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-9 main
deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-10 main
deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-10 main
EOF

apt-get update -qq

apt-get install -y --allow-unauthenticated --no-install-recommends \
    clang-3.5 \
    clang-3.6 \
    clang-3.7 \
    clang-3.8 \
    clang-3.9 \
    clang-4.0 \
    clang-5.0 \
    clang-6.0 \
    clang-7 \
    clang-8 \
    clang-9 \
    clang-10 \
    g++-5 \
    g++-4.9 \
    g++-6 \
    python \
    git \
    clang-tidy \
    clang-format

pip3 install typed_ast
