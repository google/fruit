#!/bin/bash

set -e

install_brew_package() {
  if brew list -1 | grep -q "^$1\$"; then
    # Package is installed, upgrade if needed
    brew outdated "$1" || brew upgrade "$@"
  else
    # Package not installed yet, install.
    brew install "$@"
  fi
}

install_brew_package gcc48
install_brew_package valgrind
# Note: the lack of quotes is intentional to allow passing options (e.g. "--with-clang" inside COMPILER_TO_INSTALL.
install_brew_package ${COMPILER_TO_INSTALL}
