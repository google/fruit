#!/bin/bash

rm -rf coverage
mkdir coverage
cd coverage
mkdir binaries

COMPILE_COMMAND=(g++ -O0 -fno-exceptions -DNDEBUG -W -Wall -Werror -std=c++11 -fprofile-arcs -fno-exceptions -ftest-coverage -I../include)
FRUIT_OBJS=()


for s in $(cd ../src; echo *.cpp)
do
   "${COMPILE_COMMAND[@]}" -c ../src/"$s" -o "${s/.cpp/.o}" &
   FRUIT_OBJS+=("${s/.cpp/.o}")
done

wait

for t in $(cd ../tests; ls -1 *.cpp | fgrep -v include_test.cpp)
do
  fgrep -q expect-compile-error ../tests/"$t" || \
  "${COMPILE_COMMAND[@]}" ../tests/"$t" ${FRUIT_OBJS[@]} -o binaries/${t/.cpp/} &
done

wait

for b in binaries/*
do
  ./"$b" || true
done &>/dev/null

PROJECT_DIR="$(cd ..; echo $PWD)"

lcov --rc lcov_branch_coverage=1 -capture --directory . --output-file all-coverage.info
lcov --rc lcov_branch_coverage=1 --base-directory "$PROJECT_DIR" \
     --extract all-coverage.info "$PROJECT_DIR/src/*" \
     --extract all-coverage.info "$PROJECT_DIR/include/*" \
     --output-file coverage.info
genhtml --branch-coverage --demangle-cpp coverage.info --output-directory html

xdg-open html/index.html
