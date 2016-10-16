#!/bin/bash

set -e

# python3-sh doesn't exist in Ubuntu 14.04, we must install it via pip instead.
pip3 install --user sh
