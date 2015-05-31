#!/bin/bash

set -e

run_make() {
  make -j2 VERBOSE=1
}

build_codebase() {
  cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCOMPILERS_TO_TEST=${CXX} -DCMAKE_CXX_FLAGS="${CXXFLAGS}"
  run_make
  
  cd examples
  run_make
  cd ..
}

run_tests() {
  cd tests
  run_make
  ctest --output-on-failure -j2
  cd ..
}

mkdir build
cd build

echo "==================================================="
echo "=          Debug mode (compile + test)            ="
echo "==================================================="
export BUILD_TYPE="Debug"
export CXXFLAGS="-O2 -W -Wall -Werror"
build_codebase
run_tests

echo "==================================================="
echo "=    Release mode with DNDEBUG (compile only)     ="
echo "==================================================="
export BUILD_TYPE="Release"
export CXXFLAGS="-O2 -W -Wall -Werror -DNDEBUG"
build_codebase

echo "==================================================="
echo "=  Release mode without DNDEBUG (compile + test)  ="
echo "==================================================="
export BUILD_TYPE="Release"
export CXXFLAGS="-O2 -W -Wall -Werror"
build_codebase
run_tests
