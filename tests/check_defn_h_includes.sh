#!/bin/bash

RESULT=0

for DEF in $(find fruit/ | egrep '\.defn.h')
do
    MAIN_HEADERS=($(fgrep -Rl "#include <$DEF>" fruit/));
    if [ ${#MAIN_HEADERS[@]} != 1 ]
    then
        echo "Found !=1 main headers for $DEF:"
        ls -1 "${MAIN_HEADERS[@]}" | sed 's/^/    /'
        echo
        RESULT=1
        continue
    fi
    MAIN_HEADER="${MAIN_HEADERS[0]}"
    # MAIN_HEADER includes DEF, but we need to check that DEF includes MAIN_HEADER too.
    if ! fgrep -q "#include <$MAIN_HEADER>" "$DEF"
    then
        echo "$DEF should include $MAIN_HEADER" 
        echo
        RESULT=1
        continue
    fi
done

exit $RESULT
