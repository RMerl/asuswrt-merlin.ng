#
# Copyright (C) 2024, Broadband Forum
# Copyright (C) 2024  Vantiva
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

#
# file Dockerfile
#
# Used to create a docker image running OBUSPA
# Optionally may also be used to generate a build environment for building OBUSPA
# NOTE: This Dockerfile may be used standalone from the rest of the OBUSPA repository

# Basic Usage
# Create a container with latest version of OBUSPA on GitHub and all library dependencies:
#   docker build --no-cache -f Dockerfile -t obuspa:latest .
#
# NOTE: The --no-cache option ensures that the container always builds
#       the latest version of OBUSPA and libwebsockets
#
# Running the container with default factory reset database and arguments:
#   docker run --init obuspa:latest
#
# NOTE: The --init option ensures that CTRL-C may be used to kill the container
#

# Advanced Usage
# Multi-stage build:
# 1) Build OBUSPA and all necessary library dependencies:
#   docker build --no-cache -f Dockerfile -t obuspa:build-env --target build-stage .
# 2) Create a container with OBUSPA and all library dependencies
#   docker build --no-cache -f Dockerfile -t obuspa:latest --target exec-stage .
#
# Running the container with factory reset file mapped into the container from
# the host, and providing the command line arguments to use when invoking OBUSPA:
#   docker run --init -v [host-dir]:/usr/local/etc obuspa:latest -p -v4 -r /usr/local/etc/factory_reset.txt -f /tmp/usp.db
# where [host-dir] is a directory on the host containing the factory reset file
#
# Same as last, but additionally preserving the database between docker runs:
#   docker run --init -v [host-dir]:/usr/local/etc obuspa:latest -p -v4 -r /usr/local/etc/factory_reset.txt -f /usr/local/etc/usp.db
#
# Building OBUSPA in the build environment:
#   docker run --init -it obuspa:build-env
#   cd /usr/local/src/obuspa
#   make
#

FROM debian:stable AS build-stage

RUN apt update && apt -y install \
    build-essential autoconf automake libtool pkg-config cmake git \
    libssl-dev libcurl4-openssl-dev libsqlite3-dev libz-dev libmosquitto-dev

RUN mkdir -p /usr/local/src
WORKDIR /usr/local/src/
RUN git clone https://github.com/warmcat/libwebsockets.git libwebsockets
RUN cd libwebsockets && cmake . && make && make install
RUN ldconfig

RUN git clone https://github.com/BroadbandForum/obuspa.git obuspa
RUN cd obuspa && autoreconf --force --install && ./configure && make install

FROM debian:stable AS exec-stage
RUN apt update && apt -y install libssl-dev libsqlite3-dev libcurl4-openssl-dev libmosquitto-dev

WORKDIR /
COPY --from=build-stage /usr/local/lib/libwebsockets.* /usr/local/lib/
COPY --from=build-stage /usr/local/bin/obuspa /bin
COPY --from=build-stage /usr/local/src/obuspa/factory_reset_example.txt /etc
RUN ldconfig

ENTRYPOINT ["/bin/obuspa"]
CMD ["-p", "-v4", "-r", "/etc/factory_reset_example.txt", "-f", "/tmp/usp.db"]
