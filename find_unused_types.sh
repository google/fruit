#!/bin/bash

# This script greps the source code trying to find unused types.

for I in $(egrep -R '^ *(class|struct) ' include/ src/ | sed -e 's/.*class \([a-zA-Z0-9_]*\).*/\1/;s/.*struct \([a-zA-Z0-9_]*\).*/\1/' | sort | uniq)
do
  N=$(fgrep -R "$I" include/ src/ | egrep -v '.*: *(class|struct) '"$I" | wc -l)
  if [ $N == 0 ]
  then
    echo "$I"
    fgrep -R "$I" include/ src/
    echo
  fi
done
