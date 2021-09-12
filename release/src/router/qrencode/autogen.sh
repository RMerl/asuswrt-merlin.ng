#!/bin/sh

set -e

if [ `uname -s` = Darwin ]; then
    LIBTOOLIZE=glibtoolize
else
    LIBTOOLIZE=libtoolize
fi

ACLOCAL_OPT=""
if [ -d /usr/local/share/aclocal ]; then
    ACLOCAL_OPT="-I /usr/local/share/aclocal"
elif [ -d /opt/local/share/aclocal ]; then
    ACLOCAL_OPT="-I /opt/local/share/aclocal"
elif [ -d /usr/share/aclocal ]; then
    ACLOCAL_OPT="-I /usr/share/aclocal"
fi

if [ ! -d use ]; then
    mkdir use
fi

if [ ! -d m4 ]; then
    mkdir m4
fi

autoheader

aclocal $ACLOCAL_OPT

$LIBTOOLIZE --automake --copy
automake --add-missing --copy

autoconf
