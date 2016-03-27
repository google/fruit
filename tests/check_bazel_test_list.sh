#!/bin/bash

if [ "$#" != 1 ]
then
    echo "Usage: $0 test_lists.bzl"
    exit 1
fi

F=$(mktemp)

OLDPWD="$PWD"
cd "$(dirname $1)"
$OLDPWD/tests/compute_bazel_test_lists_copy.sh >$F
cd "$OLDPWD"

if ! cmp "$F" "$1";
then
    echo "The test_lists.bzl file in this directory is out of date."
    echo "You need to re-run compute_bazel_test_lists.sh >test_lists.bzl in this directory."
    echo "Mismatches found:"
    diff -Naur "$F" "$1"
    exit 1
fi
