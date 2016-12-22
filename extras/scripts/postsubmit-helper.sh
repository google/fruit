#!/bin/bash -x

set -e

# This only exists in OS X, but it doesn't cause issues in Linux (the dir doesn't exist, so it's
# ignored).
export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"

case $COMPILER in
gcc-4.8)
    export CC=gcc-4.8
    export CXX=g++-4.8 
    ;;
    
gcc-4.9)
    export CC=gcc-4.9
    export CXX=g++-4.9
    ;;
    
gcc-5)
    export CC=gcc-5
    export CXX=g++-5
    ;;
    
gcc-6)
    export CC=gcc-6
    export CXX=g++-6
    ;;
    
clang-3.5)
    export CC=clang-3.5
    export CXX=clang++-3.5
    ;;

clang-3.6)
    export CC=clang-3.6
    export CXX=clang++-3.6
    ;;

clang-3.7)
    export CC=clang-3.7
    export CXX=clang++-3.7
    ;;

clang-3.8)
    export CC=clang-3.8
    export CXX=clang++-3.8
    ;;
    
clang-3.9)
    export CC=clang-3.9
    export CXX=clang++-3.9
    ;;

clang-default)
    export CC=clang
    export CXX=clang++
    ;;

bazel)
    ;;

*)
    echo "Unrecognized value of COMPILER: $COMPILER"
    exit 1
esac

run_make() {
  make -j$N_JOBS
}

if [[ "${COMPILER}" != "bazel" ]]
then
    echo CXX version: $($CXX --version)
    echo C++ Standard library location: $(echo '#include <vector>' | $CXX -x c++ -E - | grep 'vector\"' | awk '{print $3}' | sed 's@/vector@@;s@\"@@g' | head -n 1)
    echo Normalized C++ Standard library location: $(readlink -f $(echo '#include <vector>' | $CXX -x c++ -E - | grep 'vector\"' | awk '{print $3}' | sed 's@/vector@@;s@\"@@g' | head -n 1))

    case "$1" in
    DebugPlain)      CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Debug   -DCMAKE_CXX_FLAGS="$STLARG -O2") ;;
    DebugAsan)       CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Debug   -DCMAKE_CXX_FLAGS="$STLARG -O0 -fsanitize=address") ;;
    DebugAsanUbsan)  CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Debug   -DCMAKE_CXX_FLAGS="$STLARG -O0 -fsanitize=address,undefined")
        if [[ "${COMPILER}" =~ gcc.* ]]
        then
            # Don't use PCHs in tests if the compiler and GCC and we're testing with the undefined behavior sanitizer.
            # This combination doesn't work in GCC <6.3.0, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66343.
            CMAKE_ARGS+=(-DFRUIT_TESTS_USE_PRECOMPILED_HEADERS=OFF)
        fi
        ;;
    DebugValgrind)   CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Debug   -DCMAKE_CXX_FLAGS="$STLARG -O2"     -DRUN_TESTS_UNDER_VALGRIND=TRUE) ;;
    ReleasePlain)    CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="$STLARG -Werror") ;;
    ReleaseValgrind) CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="$STLARG -Werror" -DRUN_TESTS_UNDER_VALGRIND=TRUE) ;;
    *) echo "Error: you need to specify one of the supported postsubmit modes (see postsubmit.sh)."; exit 1 ;;
    esac

    rm -rf build
    mkdir build
    cd build
    cmake .. "${CMAKE_ARGS[@]}"
    echo
    echo "Content of CMakeFiles/CMakeError.log:"
    if [ -f "CMakeFiles/CMakeError.log" ]
    then
      cat CMakeFiles/CMakeError.log
    fi
    echo
    run_make

    cd examples
    run_make
    cd ..

    cd tests
    run_make
    ctest --output-on-failure -j$N_JOBS
else
    # COMPILER=bazel
    
    BAZEL_FLAGS=("--force_python=PY3")
    case "$1" in
    DebugPlain)      ;;
    ReleasePlain)    BAZEL_FLAGS+=("-c" "opt") ;;
    *) echo "Error: you need to specify one of the supported postsubmit modes (see postsubmit.sh)."; exit 1 ;;
    esac

    cd extras/bazel_root/third_party/fruit
    bazel build "${BAZEL_FLAGS[@]}" :fruit examples/... tests/...
    bazel test "${BAZEL_FLAGS[@]}" tests/...
fi
