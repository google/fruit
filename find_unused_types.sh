#!/bin/bash

# This script greps the source code trying to find unused types.

TYPES=($(egrep -R '^( *class| *struct|using) ' include/ src/ | sed -e 's/.*\(class\|struct\|using\) \([a-zA-Z0-9_]*\).*/\2/' | sort | uniq))

echo Candidates:
for I in ${TYPES[@]}
do
  N=$(fgrep -Rl "$I" include/ src/ | wc -l)
  if [ $N == 1 ]
  then
    echo "$I"
    grep -R "$I" include/ src/
    echo
  fi
done


echo Strong candidates:
for I in ${TYPES[@]}
do
  N=$(fgrep -R "$I" include/ src/ | egrep -v '.*: *(class|struct) '"$I" | wc -l)
  if [ $N == 0 ]
  then
    echo "$I"
    fgrep -R "$I" include/ src/
    echo
  fi
done
