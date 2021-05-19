#!/bin/sh
# Copyright 2019, The Tor Project, Inc.
# See LICENSE for licensing information

# Integration test for checkSpace.pl, which we want to rewrite.

umask 077
set -e

# Skip this test if we're running on Windows; we expect line-ending
# issues in that case.
case "$(uname -s)" in
    CYGWIN*) WINDOWS=1;;
    MINGW*) WINDOWS=1;;
    MSYS*) WINDOWS=1;;
    *) WINDOWS=0;;
esac
if test "$WINDOWS" = 1; then
    # This magic value tells automake that the test has been skipped.
    exit 77
fi

# make a safe space for temporary files
DATA_DIR=$(mktemp -d -t tor_checkspace_tests.XXXXXX)
trap 'rm -rf "$DATA_DIR"' 0

RECEIVED_FNAME="${DATA_DIR}/got.txt"

cd "$(dirname "$0")/checkspace_tests"

# we expect this to give an error code.
../checkSpace.pl -C ./*.[ch] ./*/*.[ch] > "${RECEIVED_FNAME}" && exit 1

diff -u expected.txt "${RECEIVED_FNAME}" || exit 1

echo "OK"
