#!/bin/bash

for f in *.cpp *.h
do
  echo "// $f"
  cat "$f"
  echo
  echo
done | grep -v '^/\*' | grep -v '^ \*' | grep -v '^ \*/' | grep -v '_H$'
