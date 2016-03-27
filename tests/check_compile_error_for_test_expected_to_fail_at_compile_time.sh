#!/bin/bash

if [ "$#" != 2 ]
then
    echo "Usage: $0 compilation_output_file test_source"
    exit 1
fi

TEST_NAME=$(basename $2)
TEST_NAME=${TEST_NAME/.cpp/}

if ! head -n 1 "$2" | grep -q '^// expect-compile-error '
then
    echo "check_compile_error_for_test_expected_to_fail_at_compile_time.sh called for a test source that doesn't have '// expect-compile-error': $2."
    exit 1
fi

EXPECTED_MESSAGE="$(head -n 1 "$2" | sed 's@^// expect-compile-error @@')"

ERROR_TYPE="$(cat $1 | egrep -on 'fruit::impl::(.*Error<.*>)' | head -n 1 | tr -d ' ')"
ERROR_TYPE_LINE="$(echo $ERROR_TYPE | awk -F: '{print $1}')"
ERROR_TYPE_MESSAGE="$(echo $ERROR_TYPE | sed 's/[0-9]*:fruit::impl:://')"
STATIC_ASSERT_ERROR="$(cat $1 | grep -n 'static.assert' | head -n 1)"
STATIC_ASSERT_ERROR_LINE="$(echo $STATIC_ASSERT_ERROR | awk -F: '{print $1')"
STATIC_ASSERT_ERROR_MESSAGE="$(echo $STATIC_ASSERT_ERROR | sed 's/.*static.assert failed[^A-Za-z0-9]*//;s/"//')"
EXPECTED_ERROR_TYPE="$(echo "${EXPECTED_MESSAGE}" | awk -F'|' '{print $1}')"
EXPECTED_STATIC_ASSERT_ERROR="$(echo "${EXPECTED_MESSAGE}" | awk -F'|' '{print $2}')"
if ! (echo $ERROR_TYPE_MESSAGE | grep -q "$EXPECTED_ERROR_TYPE") || ! (echo $STATIC_ASSERT_ERROR_MESSAGE | grep -q "$EXPECTED_STATIC_ASSERT_ERROR"); then
    echo 'The compilation of ${NAME} failed as expected, but with a different message.'
    echo "Expected error type: $EXPECTED_ERROR_TYPE"
    echo "Error type was:      $ERROR_TYPE_MESSAGE"
    echo "Expected static assert: $EXPECTED_STATIC_ASSERT_ERROR"
    echo "Static assert was:      $STATIC_ASSERT_ERROR_MESSAGE"
    cat $1 | head -n 40
    exit 1
fi
# 6 is just a constant that works for both g++ (<=4.8.3) and clang++ (<=3.5.0). It might need to be changed.
if [ $STATIC_ASSERT_ERROR_LINE -gt 6 ] || [ $ERROR_TYPE_LINE -gt 6 ]; then
    cat $1 | head -n 6
    echo 'The compilation of ${NAME} failed with the expected message, but the error message contained too many lines before the relevant ones.'
    echo "The error type was reported on line $ERROR_TYPE_LINE of the message (should be <=6)"
    echo "The static assert was reported on line $STATIC_ASSERT_ERROR_LINE of the message (should be <=6)"
    exit 1
fi

if cat $1 | head -n 6 | grep -q 'fruit::impl::meta'; then
    echo 'The compilation of ${NAME} failed with the expected message, but the error message contained some metaprogramming types in the output (besides Error).'
    cat $1 | head -n 40
    exit 1
fi

exit 0
