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

mkdir build
cd build

if [[ ${BUILD_TYPE} == "Debug" ]]
then
  export CXXFLAGS="-O2          -W -Wall -Werror"
else
  export CXXFLAGS="-O2 -DNDEBUG -W -Wall -Werror"
fi

build_codebase

if [[ ${BUILD_TYPE} != "Debug" ]]
then
  # Recompile everything without -DNDEBUG to prepare for test execution.
  export CXXFLAGS="-O2 -W -Wall -Werror"
  build_codebase
fi

cd tests
run_make
ctest --output-on-failure -j2
