#!/bin/bash

set -e

# Strip some binaries that aren't already stripped, to save space.
find /usr/lib/ /usr/bin -type f | fgrep -v bazel | fgrep -v python | \
    xargs -P 32 -L 1 bash -c 'file "$0" | fgrep executable | fgrep -q stripped && strip --strip-unneeded "$0" || true'

# This was only needed above, we don't need it in the final image.
apt-get remove -y wget file python3-pip
apt-get autoremove -y

# Remove temporary files, to save space.
apt-get clean
rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
