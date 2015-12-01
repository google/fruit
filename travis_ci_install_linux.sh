#!/bin/bash

set -e

# Always install latest GCC 4.8 to avoid bugs in old STL when compiling with Clang.
sudo apt-get install -qq --force-yes g++-4.8
sudo apt-get install -qq --force-yes valgrind ${COMPILER_TO_INSTALL}
