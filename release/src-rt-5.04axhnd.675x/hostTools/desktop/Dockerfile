FROM ubuntu:16.04

MAINTAINER Jing Qiu <jane.qiu@broadcom.com>

LABEL Description="This image provides a development environment for Broadcom BCA-CPE" Vendor="Broadcom" Version="1.0"

RUN \
    apt-get update \
    && apt-get install \
       --yes \
       --no-install-recommends \
       --no-install-suggests \
    build-essential \
    libncurses5-dev \
    zlib1g-dev \
    libssl-dev \
    iproute2 \
    gawk \
    python \
    subversion \
    unzip \
    liblzo2-dev \
    uuid-dev \
    autoconf \
    pkg-config \
    automake \
    ccache \
    ca-certificates \
    libtool \
    patch \
    vim \
    wget \
    bison \
    flex \
    git \
    bc \
    cpio \
    gettext \
    dos2unix \
    lsb-release \
    rsync \
    lib32z1 \
    libglib2.0-dev \
    gdisk \
    man \
    openjdk-8-jdk \
    sudo

# Enable 32-bit program execution on 64bit system
RUN \
    dpkg --add-architecture i386 \
    && apt-get update \
    && apt-get install \
       --yes \
       --no-install-recommends \
       --no-install-suggests \
    libc6:i386 libncurses5:i386 libstdc++6:i386 libc6-dev:i386 \
    libselinux1:i386 gcc-multilib g++-multilib

# Runtime debugging tools installation
RUN \
    apt update \
    && apt install \
    --yes \
    --no-install-recommends \
    --no-install-suggests \
    strace

# Use bash and prepare beepuser as USER of the container
RUN \
    rm /bin/sh \
    && ln -sf bash /bin/sh \
    && adduser --disabled-password --gecos '' --home /home/beepuser beepuser \
    && adduser beepuser sudo \
    && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers


ADD ./toolchains/* /


RUN mkdir -p /opt/linuxout && chown beepuser /opt/linuxout
RUN mkdir -p /opt/linux && chown beepuser /opt/linux
RUN mkdir -p /var/run/dbus && chown beepuser /var/run/dbus
RUN mkdir -p /var/lib/dbus && chown beepuser /var/lib/dbus

VOLUME ["/data"]

WORKDIR "/home/beepuser"

#Add ccache support
ADD ./add_cached.sh /usr/local/sbin/add_cached.sh
RUN chmod +x /usr/local/sbin/add_cached.sh
RUN mkdir /home/beepuser/ccache
RUN chown beepuser /home/beepuser/ccache
ENV CCACHE_DIR "/home/beepuser/ccache"
# Setup host side ccache, mainly for DESKTOP_LINUX (TOOLCHAIN_BASE=/usr/lib/ccache)
RUN \ 
  cd /usr/lib/ccache \
  && mkdir -p /usr/lib/ccache/usr \
  && ln -s ../../bin/ar ar \
  && ln -s ../../bin/as as \
  && ln -s ../../bin/cpp cpp \
  && ln -s ../../bin/ld ld \
  && ln -s ../../bin/nm nm \
  && ln -s ../../bin/objcopy objcopy \
  && ln -s ../../bin/objdump objdump \
  && ln -s ../../bin/ranlib ranlib \
  && ln -s ../../bin/size size \
  && ln -s ../../bin/strip strip \
  && ln -s /usr/lib/ccache/ /usr/lib/ccache/usr/bin

#Add script to build desktop linux for BEEP
ADD ./dt.sh /usr/local/sbin/dt.sh
RUN chmod +x /usr/local/sbin/dt.sh

RUN mkdir -p /home/beepuser/scripts
ADD ./scripts /home/beepuser/scripts
RUN chmod +x /home/beepuser/scripts/simulate.sh; exit 0
RUN chmod +x /home/beepuser/scripts/start.sh; exit 0

USER beepuser


# Default command.
CMD ["bash"]

