#!/bin/bash

SUCCESSFUL_TESTS=""
TESTS_EXPECTED_TO_FAIL_AT_RUNTIME=""
TESTS_EXPECTED_TO_FAIL_AT_COMPILE_TIME=""

NEWLINE=$'\n'

for F in $(ls -1 *.cpp | LC_ALL=C sort)
do
    if [ "$F" == "include_test.cpp" ]
    then
        continue
    fi
    S="        \"${F/.cpp/}\",$NEWLINE"
    case "$(head -n 1 "$F")" in
    //\ expect-success)       SUCCESSFUL_TESTS+="$S" ;;
    //\ expect-runtime-error\ *) TESTS_EXPECTED_TO_FAIL_AT_RUNTIME+="$S" ;;
    //\ expect-compile-error\ *) TESTS_EXPECTED_TO_FAIL_AT_COMPILE_TIME+="$S" ;;
    *) echo "The test $F doesn't start with a '// expect-{success,compile-error,runtime-error}' comment"; exit 1;;
    esac
done

cat <<EOF
TEST_LISTS = {
    "SUCCESSFUL_TESTS": [
    $SUCCESSFUL_TESTS],

    "TESTS_EXPECTED_TO_FAIL_AT_RUNTIME": [
    $TESTS_EXPECTED_TO_FAIL_AT_RUNTIME],

    "TESTS_EXPECTED_TO_FAIL_AT_COMPILE_TIME": [
    $TESTS_EXPECTED_TO_FAIL_AT_COMPILE_TIME],
}
EOF

