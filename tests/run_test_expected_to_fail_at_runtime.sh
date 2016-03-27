#!/bin/bash

if [ "$#" != 2 ]
then
    echo "Usage: $0 test_executable test_source"
    exit 1
fi

if ! head -n 1 "$2" | grep -q '^// expect-runtime-error '
then
    echo "run_test_expected_to_fail_at_runtime.sh called for a test source that doesn't have '// expect-runtime-error': $2."
    echo "First line is:"
    head -n 1 "$2" 
    exit 1
fi

EXPECTED_MESSAGE="$(head -n 1 "$2" | sed 's@^// expect-runtime-error @@')"

F=`mktemp`
if "$1" &>$F; then
    cat \$F | fgrep error | head -n 1
    rm -f $F
    echo 'Expected runtime error but the test passed.'
    exit 1
fi

if ! cat $F | fgrep error | head -n 1 | grep -q "$EXPECTED_MESSAGE"; then
    cat $F | fgrep error | head -n 1
    rm -f $F
    echo "The test failed as expected, but with a different message. Expected: $EXPECTED_MESSAGE"
    exit 1
fi

rm -f $F
