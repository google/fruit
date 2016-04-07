#!/bin/bash

echo "// Running from dir $PWD"
echo 'digraph g {'
for f in $(ls *.cpp *.h)
do
  echo "$(echo "$f" | sed 's/\./_/g') [label=\"$f\"]"
  for g in $(fgrep -l "#include \"$f\"" *)
  do
    echo "$g -> $f" | sed 's/\./_/g'
  done
done
echo '}'
