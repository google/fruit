#!/bin/bash

set -e

# This only exists in OS X, but it doesn't cause issues in Linux (the dir doesn't exist, so it's
# ignored).
export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"

case "$1" in
DebugAsan)       CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Debug   -DCMAKE_CXX_FLAGS="-O0"     -DINSTRUMENT_WITH_SANITIZERS=TRUE) ;;
DebugValgrind)   CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Debug   -DCMAKE_CXX_FLAGS="-O2"     -DRUN_TESTS_UNDER_VALGRIND=TRUE) ;;
ReleasePlain)    CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Werror") ;;
ReleaseValgrind) CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Werror" -DRUN_TESTS_UNDER_VALGRIND=TRUE) ;;
*) echo "Error: you need to specify one of the supported postsubmit modes (see postsubmit.sh)." ;;
esac

run_make() {
  make -j2 VERBOSE=1
}

rm -rf build
mkdir build
cd build
cmake .. "${CMAKE_ARGS[@]}"
run_make

cd examples
run_make
cd ..

cd tests
run_make
ctest --output-on-failure -j2
