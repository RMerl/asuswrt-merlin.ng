#!/bin/sh
# Copyright (c) 2026, The Tor Project, Inc.
# See LICENSE for licensing information.

set -eu
umask 077

# This test needs Python 3 and Unix-domain control sockets.
if ! "${PYTHON:-python3}" -c '
import sys, socket
sys.exit(0 if sys.version_info >= (3, 7) and hasattr(socket, "AF_UNIX") else 1)
'; then
  echo "CBT integration requires Python 3.7 and Unix sockets. Skipping."
  exit 77
fi

exec "${PYTHON:-python3}" "${abs_top_srcdir:-.}/src/test/test_cbt_mvp.py" \
  "${TESTING_TOR_BINARY:-./src/app/tor}"
