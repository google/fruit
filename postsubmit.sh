#!/bin/bash

set -e

mkdir build
cd build

EXTRA_CMAKE_ARGS=()
if [[ ${BUILD_TYPE} != "Debug" ]]
then
  EXTRA_CMAKE_ARGS+=(-DCMAKE_CXX_FLAGS="-O2 -DNDEBUG -W -Wall -Werror")
fi

cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ${EXTRA_CMAKE_ARGS[@]}

make -j2 VERBOSE=1

cd tests
if [[ ${BUILD_TYPE} == "Debug" ]]
then
  ctest --output-on-failure -j2
else
  # No need to execute the tests, but check that they still compile.
  make -j2 VERBOSE=1
fi
