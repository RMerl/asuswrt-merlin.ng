#
# file Dockerfile
#
# Base docker image with all required dependencies for OB-USP-A
#
# Based on Ubuntu 22.10 (Kinetic Kudu), which provides libmosquitto 2.0.11 and libwebsockets 4.1.6
# This image includes some basic compilation tools (automake, autoconf)
#
# One-liner execution line (straightforward build for OB-USP-A execution):
# > docker build -f Dockerfile -t obuspa:latest .
#
# Multi-stage builds execution lines (to tag build stages):
# 1) Create the build environment image:
# > docker build -f Dockerfile -t obuspa:build-env --target build-env .
# 2) Create the OB-USP-A image, then build the application
# > docker build -f Dockerfile -t obuspa:latest --target runner .
#
FROM ubuntu:kinetic AS build-env

# Install dependencies
RUN apt-get update && apt-get -y install \
        libssl-dev \
        libcurl4-openssl-dev\
        libsqlite3-dev \
        libz-dev \
        autoconf \
        automake \
        libtool \
        libmosquitto-dev \
        libwebsockets-dev \
        pkg-config \
        make \
    && apt-get clean

FROM build-env AS runner

ENV MAKE_JOBS=8
ENV OBUSPA_ARGS="-v4"

# Copy in all of the code
# Then compile, as root.
COPY . /obuspa/
RUN cd /obuspa/ && \
    autoreconf -fi && \
    ./configure && \
    make -j${MAKE_JOBS} && \
    make install

# Then delete the code
# that's no longer needed
RUN rm -rf /obuspa

# Run obuspa with args expanded
CMD obuspa ${OBUSPA_ARGS}
