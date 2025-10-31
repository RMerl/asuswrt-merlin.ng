#! /bin/sh
set -x -e
# --no-recursive is available only in recent autoconf versions
touch NEWS
touch README
autoreconf -fv --install
