FROM ubuntu:16.04
MAINTAINER Marco Poletti <poletti.marco@gmail.com>

COPY ubuntu-16.04_custom.list /etc/apt/sources.list.d/
COPY common_install.sh common_cleanup.sh ubuntu-16.04_install.sh /

RUN bash -x /common_install.sh && \
    bash -x /ubuntu-16.04_install.sh && \
    bash -x /common_cleanup.sh
