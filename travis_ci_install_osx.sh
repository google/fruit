#!/bin/bash

set -e

brew install gcc48
brew install valgrind
# Note: the lack of quotes is intentional to allow passing options (e.g. "--with-clang" inside COMPILER_TO_INSTALL.
brew install ${COMPILER_TO_INSTALL}
