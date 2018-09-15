FROM multiarch/ubuntu-core:arm64-xenial
MAINTAINER Marco Poletti <poletti.marco@gmail.com>

COPY ubuntu_arm-16.04_custom.list /etc/apt/sources.list.d/
COPY common_install.sh common_cleanup.sh ubuntu_arm-16.04_install.sh /

RUN bash -x /common_install.sh && \
    bash -x /ubuntu_arm-16.04_install.sh && \
    bash -x /common_cleanup.sh
