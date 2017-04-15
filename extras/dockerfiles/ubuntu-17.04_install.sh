#!/bin/bash

set -e

apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1E9377A2BA9EF27F

apt-get install -y --allow-unauthenticated --no-install-recommends \
    g++-6 \
    python \
    python3-sh
