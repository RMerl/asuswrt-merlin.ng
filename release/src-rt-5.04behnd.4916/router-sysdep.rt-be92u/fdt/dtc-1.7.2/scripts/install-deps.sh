#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later

if [ -f /etc/os-release ]
then
    . /etc/os-release
else
    echo "ERROR: OS name is not provided."
    exit 1
fi

if [ "$NAME" = "Arch Linux" ]
then
    pacman -Syu --needed --noconfirm bison diffutils flex gcc git libyaml \
    make meson pkgconf python python-setuptools-scm swig valgrind which
elif [ "$NAME" = "Alpine Linux" ]
then
    apk add build-base bison coreutils flex git yaml yaml-dev python3-dev \
    meson py3-setuptools_scm swig valgrind
elif [ "$NAME" = "Fedora Linux" ]
then
    dnf install -y bison diffutils flex gcc git libyaml libyaml-devel \
    make meson python3-devel python3-setuptools swig valgrind which
elif [ "$NAME" = "Ubuntu" ]
then
    apt update
    apt install -yq build-essential bison flex git libyaml-dev pkg-config \
    meson python3-dev python3-setuptools python3-setuptools-scm swig valgrind
else
    echo "ERROR: OS name is not provided."
    exit 1
fi
