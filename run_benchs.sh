#!/bin/bash

set -e
set -o pipefail

prefix_with() (
  local PREFIX=("$@")
  while read line
  do
    echo "${PREFIX[@]}" "$line"
  done
)

progress() (
  local EXPECTED_LINES=$1
  local I=0
  echo "0/$EXPECTED_LINES (0%)" >&2
  while read line
  do
    echo "$line"
    I="$[$I + 1]"
    echo "$I/$EXPECTED_LINES ($[100 * $I / $EXPECTED_LINES]%)" >&2
  done
)

print_stats() (
  local TOTAL=0
  local N=0
  for VALUE in "$@"
  do
    TOTAL="$(echo $TOTAL + $VALUE | bc -l)"
    N="$[$N + 1]"
  done
  if [[ $N == 0 ]]
  then
    echo "N/A ()"
  else
    echo "$(echo $TOTAL / $N | bc -l) ($@)"
  fi
)

BASEDIR="$PWD"
COMPILERS=(g++ clang++)
# NUM_ITERATIONS = ITERATIONS_FACTOR/NUM_BINDINGS
ITERATIONS_FACTOR="$[400 * 1000 * 1000]"
# Must be multiples of 10
NUM_BINDINGS_FOR_RUNTIME_TESTS=(100 1000)
# Must be multiples of 5
NUM_BINDINGS_FOR_COMPILE_TESTS=(20 80 320)
NUM_LINES="$[${#COMPILERS[@]} * (${#NUM_BINDINGS_FOR_RUNTIME_TESTS[@]} * 3 + ${#NUM_BINDINGS_FOR_COMPILE_TESTS[@]})]"

# All result lines are of the form:
# <compiler> <n> <test> <avg. time> (<time>...)
for compiler in ${COMPILERS[@]}
do
  rm -rf build
  mkdir build
  (
    cd build
    cmake .. -DCMAKE_CXX_COMPILER=$(which $compiler) -DCMAKE_BUILD_TYPE=Release &>/dev/null
    (
      cd examples/benchmark
      for N in ${NUM_BINDINGS_FOR_RUNTIME_TESTS[@]}
      do
        (
          NUM_ITERATIONS="$[$ITERATIONS_FACTOR / $N]"
          sed -i "s/num_components_with_no_deps = .*/num_components_with_no_deps = $[$N / 10];/" $BASEDIR/examples/benchmark/generate_benchmark.cpp
          sed -i "s/num_components_with_deps    = .*/num_components_with_deps    = $[9 * ($N / 10)];/" $BASEDIR/examples/benchmark/generate_benchmark.cpp
          make benchmark &>/dev/null
          SETUP_TIMES=()
          REQUEST_TIMES=()
          for i in $(seq 1 4)
          do
            RESULTS=($(echo $NUM_ITERATIONS | ./main $NUM_ITERATIONS | fgrep Total | awk '{print $5}'))
            SETUP_TIMES+=("${RESULTS[0]}")
            REQUEST_TIMES+=("${RESULTS[1]}")
          done
          print_stats "${SETUP_TIMES[@]}" | prefix_with "fruit_setup_time"
          print_stats "${REQUEST_TIMES[@]}" | prefix_with "fruit_request_time"
          sed -i "s/#define MULTIPLIER .*/#define MULTIPLIER $N/" $BASEDIR/examples/benchmark/new_delete_benchmark.cpp
          make new_delete_benchmark &>/dev/null
          NEW_DELETE_TIMES=()
          for i in $(seq 1 4)
          do
            NEW_DELETE_TIMES+=($(echo $NUM_ITERATIONS | ./new_delete_benchmark | awk '{print $3}'))
          done 
          print_stats "${NEW_DELETE_TIMES[@]}" | prefix_with "new_delete_time"
        ) | prefix_with $N
      done
    )
    (
      cd examples/compile_time_benchmark
      for N in ${NUM_BINDINGS_FOR_COMPILE_TESTS[@]}
      do
        (
          sed -i "s/#define MULTIPLIER .*/#define MULTIPLIER $[$N/5]/" $BASEDIR/examples/compile_time_benchmark/module.cpp
          COMPILE_TIMES=()
          for i in $(seq 1 4)
          do
            COMPILE_TIMES+=($(make compile_time_benchmark 2>&1 | fgrep real | awk '{print $2}' | tr -d s | sed 's/m/*60+/' | bc))
          done
          print_stats "${COMPILE_TIMES[@]}" | prefix_with "fruit_compile_time"
        ) | prefix_with "$[$N * 5]"
      done
    )
  ) | prefix_with $compiler
done | progress $NUM_LINES
