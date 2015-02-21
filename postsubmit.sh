#!/bin/bash

set -e

mkdir build
cd build

EXTRA_CMAKE_ARGS=()
if [[ ${BUILD_TYPE} != "Debug" ]]
then
  EXTRA_CMAKE_ARGS+=(-DCMAKE_CXX_FLAGS="-O2 -DNDEBUG -W -Wall -Werror")
fi

cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} "${EXTRA_CMAKE_ARGS[@]}"

make -j2 VERBOSE=1

# TODO: Run the tests in release mode too, once Fruit starts using a real testing framework (not just assert).
cd tests
if [[ ${BUILD_TYPE} == "Debug" ]]
then
  ctest --output-on-failure -j2
fi
