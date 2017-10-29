#!/bin/bash

set -e

# Strip some binaries that aren't already stripped, to save space.
for f in $(find /usr/lib/ /usr/bin -type f | fgrep -v bazel | fgrep -v python)
do
  if file "$f" | fgrep 'executable' | fgrep -q 'stripped'
  then
    strip --strip-unneeded $f
  fi
done

# This was only needed above, we don't need it in the final image.
apt-get remove -y wget file python3-pip
apt-get autoremove -y

# Remove temporary files, to save space.
apt-get clean
rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
