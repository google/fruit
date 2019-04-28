#!/bin/bash -x

set -e

install_brew_package() {
  if brew list -1 | grep -q "^$1\$"; then
    # Package is installed, upgrade if needed
    time (brew outdated "$1" || brew upgrade "$@")
  else
    # Package not installed yet, install.
    # If there are issues, try upgrading instead.
    time (brew install "$@" || brew upgrade "$@")
  fi
}

time brew update

# For md5sum, timeout
install_brew_package coreutils

if [[ "${INSTALL_VALGRIND}" == "1" ]]
then
    install_brew_package valgrind
fi

which cmake &>/dev/null || install_brew_package cmake

case "${COMPILER}" in
gcc-4.9)       install_brew_package gcc@4.9 ;;
gcc-5)         install_brew_package gcc@5 ;;
gcc-6)         install_brew_package gcc@6 ;;
gcc-7)         install_brew_package gcc@7 ;;
gcc-8)         install_brew_package gcc@8 ;;
clang-default) ;;
clang-3.9)     install_brew_package llvm@3.9 --with-clang --with-libcxx;;
clang-4.0)     install_brew_package llvm@4   --with-clang --with-libcxx;;
clang-5.0)     install_brew_package llvm@5   --with-clang --with-libcxx;;
clang-6.0)     install_brew_package llvm@6   --with-clang --with-libcxx;;
clang-7.0)     install_brew_package llvm@7   --with-clang --with-libcxx;;
clang-8.0)     install_brew_package llvm@8   --with-clang --with-libcxx;;
*) echo "Compiler not supported: ${COMPILER}. See travis_ci_install_osx.sh"; exit 1 ;;
esac

install_brew_package boost
install_brew_package python
time pip3 install pytest
time pip3 install pytest-xdist
time pip3 install sh

# TODO: remove this.
ls -l /usr/local/include
find /usr/local/include/boost*

# This adds python-installed executables to PATH (notably py.test).
export PATH="$(brew --prefix)/bin:$PATH"
