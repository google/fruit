#!/bin/bash

echo "// Running from dir $PWD"
echo 'digraph g {'
for f in *.cpp *.h
do
  for g in $(fgrep -l "#include \"$f\"" *)
  do
    echo "$g -> $f" | sed 's/\./_/g'
  done
done
echo '}'
