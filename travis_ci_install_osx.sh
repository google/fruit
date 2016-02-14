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

case "${CXX}" in
g++)         ;;
g++-4.8)     install_brew_package homebrew/versions/gcc48 ;;
g++-4.9)     install_brew_package homebrew/versions/gcc49 ;;
g++-5)       install_brew_package homebrew/versions/gcc5 ;;
clang++)     ;;
clang++-3.5) install_brew_package homebrew/versions/llvm35 --with-clang --with-libcxx;;
clang++-3.6) install_brew_package homebrew/versions/llvm36 --with-clang --with-libcxx;;
*) echo "Compiler not supported: ${CXX}. See travis_ci_install_osx.sh"; exit 1 ;;
esac
