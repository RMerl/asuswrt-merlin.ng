#!/bin/sh

umask 077
set -e

if [ $# -ge 1 ]; then
  TOR_BINARY="${1}"
  shift
else
  TOR_BINARY="${TESTING_TOR_BINARY:-./src/app/tor}"
fi

echo "TOR BINARY IS ${TOR_BINARY}"

die() { echo "$1" >&2 ; exit 5; }

echo "A"

DATA_DIR=$(mktemp -d -t tor_cmdline_tests.XXXXXX)
trap 'rm -rf "$DATA_DIR"' 0

# 1. Test list-torrc-options.
OUT="${DATA_DIR}/output"

echo "B"
"${TOR_BINARY}" --list-torrc-options > "$OUT"

echo "C"

# regular options are given.
grep -i "SocksPort" "$OUT" >/dev/null || die "Did not find SocksPort"


echo "D"

# unlisted options are given, since they do not have the NOSET flag.
grep -i "__SocksPort" "$OUT" > /dev/null || die "Did not find __SocksPort"

echo "E"

# unsettable options are not given.
if grep -i "DisableIOCP" "$OUT"  /dev/null; then
    die "Found DisableIOCP"
fi
if grep -i "HiddenServiceOptions" "$OUT" /dev/null ; then
    die "Found HiddenServiceOptions"
fi
echo "OK"
