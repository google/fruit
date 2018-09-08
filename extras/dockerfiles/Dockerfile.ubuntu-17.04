FROM ubuntu:17.04
MAINTAINER Marco Poletti <poletti.marco@gmail.com>

COPY ubuntu-17.04_custom.list /etc/apt/sources.list.d/
COPY common_install.sh common_cleanup.sh ubuntu-17.04_install.sh /

RUN sed -i -re 's/([a-z]{2}\.)?archive.ubuntu.com|security.ubuntu.com/old-releases.ubuntu.com/g' /etc/apt/sources.list; \
    bash -x /common_install.sh && \
    bash -x /ubuntu-17.04_install.sh && \
    bash -x /common_cleanup.sh
