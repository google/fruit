#!/bin/bash -x

set -e

install_brew_package() {
  if brew list -1 | grep -q "^$1\$"; then
    # Package is installed, upgrade if needed
    brew outdated "$1" || brew upgrade "$@"
  else
    # Package not installed yet, install.
    # If there are conflicts, try overwriting the files (these are in /usr/local anyway so it should be ok).
    brew install "$@" || brew link --overwrite gcc49
  fi
}

# For md5sum
install_brew_package md5sha1sum
install_brew_package gcc48
# For `timeout'
install_brew_package coreutils
install_brew_package valgrind
# Note: the lack of quotes is intentional to allow passing options (e.g. "--with-clang" inside COMPILER_TO_INSTALL.
install_brew_package ${COMPILER_TO_INSTALL}
