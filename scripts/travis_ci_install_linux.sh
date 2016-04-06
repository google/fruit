#!/bin/bash -x

set -e

# Always install latest GCC 4.8 to avoid bugs in old STL when compiling with Clang.
PACKAGES=(g++-4.8 valgrind ${CXX})

if [ "$STL" == "libc++" ]
then
    PACKAGES+=(libc++-dev)
fi

for P in ${PACKAGES[@]}
do
   docker exec apt-get install -qq --force-yes "$P"
done
