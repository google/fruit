#!/bin/bash -x

set -e

# These packages depend on the ones that we update but we don't care about these, we don't want to waste time upgrading
# them.
for p in postgis ansible libdap libspatialite gdal mercurial poppler
do
  brew pin $p
done

install_brew_package() {
  time (brew install "$@" || brew outdated "$1" || brew upgrade "$@" || true)
  # Some formulas are not linked into /usr/local by default, make sure they are.
  time (brew unlink "$@" || true)
  time (brew link "$@" || true)
}

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
gcc-9)         install_brew_package gcc@9 ;;
gcc-10)         install_brew_package gcc@10 ;;
gcc-11)         install_brew_package gcc@11 ;;
gcc-12)         install_brew_package gcc@12 ;;
clang-default) ;;
clang-6.0)
  install_brew_package llvm@6
  ln -s /usr/local/opt/llvm@6/bin/clang++ /usr/local/bin/clang++-6.0
  ;;
clang-7.0)
  install_brew_package llvm@7
  ln -s /usr/local/opt/llvm@7/bin/clang++ /usr/local/bin/clang++-7
  ;;
clang-8.0)
  install_brew_package llvm@8
  ln -s /usr/local/opt/llvm@8/bin/clang++ /usr/local/bin/clang++-8
  ;;
clang-9.0)
  install_brew_package llvm@9
  ln -s /usr/local/opt/llvm@9/bin/clang++ /usr/local/bin/clang++-9
  ln -s /usr/local/opt/llvm@9/bin/clang /usr/local/bin/clang-9
  ;;
clang-11.0)
  install_brew_package llvm@11
  ln -s /usr/local/opt/llvm@11/bin/clang++ /usr/local/bin/clang++-11
  ;;
clang-12.0)
  install_brew_package llvm@12
  ln -s /usr/local/opt/llvm@12/bin/clang++ /usr/local/bin/clang++-12
  ;;
clang-13.0)
  install_brew_package llvm@13
  ln -s /usr/local/opt/llvm@13/bin/clang++ /usr/local/bin/clang++-13
  ;;
clang-14.0)
  install_brew_package llvm@14
  ln -s /usr/local/opt/llvm@14/bin/clang++ /usr/local/bin/clang++-14
  ;;
clang-15.0)
  install_brew_package llvm@15
  ln -s /usr/local/opt/llvm@15/bin/clang++ /usr/local/bin/clang++-15
  ;;
*) echo "Compiler not supported: ${COMPILER}. See travis_ci_install_osx.sh"; exit 1 ;;
esac

install_brew_package boost
install_brew_package python
time pip3 install absl-py
time pip3 install pytest
time pip3 install pytest-xdist
time pip3 install sh
