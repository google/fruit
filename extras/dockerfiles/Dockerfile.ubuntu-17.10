FROM ubuntu:17.10
MAINTAINER Marco Poletti <poletti.marco@gmail.com>

COPY ubuntu-17.10_custom.list /etc/apt/sources.list.d/
COPY common_install.sh common_cleanup.sh ubuntu-17.10_install.sh /

RUN bash -x /common_install.sh && \
    bash -x /ubuntu-17.10_install.sh && \
    bash -x /common_cleanup.sh
