FROM ubuntu:15.10
MAINTAINER Marco Poletti <poletti.marco@gmail.com>

COPY custom.list /etc/apt/sources.list.d/

# 1E9377A2BA9EF27F is the key for the ubuntu-toolchain-r PPA.
RUN apt-get install -y --no-install-recommends wget && \
    wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | apt-key add - && \
    apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1E9377A2BA9EF27F && \
    apt-get update -qq && \
    apt-get install -y --no-install-recommends \
        valgrind \
        make \
        cmake \
        g++-4.8 \
        g++-4.9 \
        g++-5 \
        clang-3.5 \
        clang-3.6 \
        clang-3.7 \
        clang-3.8 \
        libc++-dev && \
    for f in $(find /usr/lib /usr/bin -type f); do file "$f" | fgrep executable | fgrep -q "not stripped" && strip "$f"; done
