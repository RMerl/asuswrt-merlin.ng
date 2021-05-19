#!/usr/bin/env bash

set -e

TOR="${abs_top_builddir:-.}/src/app/tor"

INCLUDES_PY="${abs_top_srcdir:-.}/scripts/maint/practracker/includes.py"

if ! test -x "${INCLUDES_PY}" ; then
    echo "skip"
    exit 77
fi

"${TOR}" --dbg-dump-subsystem-list | \
    "${PYTHON:-python}" \
    "${INCLUDES_PY}" --check-subsystem-order - "${abs_top_srcdir}/src"

echo ok
