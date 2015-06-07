#!/bin/bash

set -e

run_make() {
  make -j2 VERBOSE=1
}

build_codebase() {
  cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_CXX_FLAGS="${CXXFLAGS}" "$@"
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

echo "======================"
echo "=     Debug mode     ="
echo "======================"
export BUILD_TYPE="Debug"
export CXXFLAGS=""
build_codebase
run_tests

echo "======================"
echo "=    Release mode    ="
echo "======================"
export BUILD_TYPE="Release"
export CXXFLAGS="-Werror"
if [[ ${RUN_RELEASE_TESTS_UNDER_VALGRIND} == 1 ]]
then
  build_codebase
else
  build_codebase -DRUN_TESTS_UNDER_VALGRIND=FALSE
fi
run_tests
