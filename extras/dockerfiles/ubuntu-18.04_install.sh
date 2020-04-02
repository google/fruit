#!/bin/bash

set -e

apt-get install -y --no-install-recommends \
    curl

# For the Bazel repository
curl https://bazel.build/bazel-release.pub.gpg | apt-key add -

echo 'deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8' >> /etc/apt/sources.list.d/bazel.list

cat <<EOF >/etc/apt/sources.list.d/custom.list
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic main
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-5.0 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-5.0 main
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-6.0 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-6.0 main
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-7 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-7 main
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-8 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-8 main
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main
EOF

apt-get update -qq

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-8 \
    g++-7 \
    g++-6 \
    g++-5 \
    clang-3.9 \
    clang-4.0 \
    clang-5.0 \
    clang-6.0 \
    clang-7 \
    clang-8 \
    clang-9 \
    clang-10 \
    bazel \
    git \
    python \
    python3-sh \
    python3-typed-ast \
    clang-tidy \
    clang-format
