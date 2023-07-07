#!/bin/sh

umask 077
set -e
set -x

# emulate realpath(), in case coreutils or equivalent is not installed.
abspath() {
    f="$*"
    if [ -d "$f" ]; then
        dir="$f"
        base=""
    else
        dir="$(dirname "$f")"
        base="/$(basename "$f")"
    fi
    dir="$(cd "$dir" && pwd)"
    echo "$dir$base"
}

UNAME_OS=$(uname -s | cut -d_ -f1)
if test "$UNAME_OS" = 'CYGWIN' || \
   test "$UNAME_OS" = 'MSYS' || \
   test "$UNAME_OS" = 'MINGW'; then
  if test "$APPVEYOR" = 'True'; then
    echo "This test is disabled on Windows CI, as it requires firewall exemptions. Skipping." >&2
    exit 77
  fi
fi

# find the tor binary
if [ $# -ge 1 ]; then
  TOR_BINARY="${1}"
  shift
else
  TOR_BINARY="${TESTING_TOR_BINARY:-./src/app/tor}"
fi

TOR_BINARY="$(abspath "$TOR_BINARY")"

echo "TOR BINARY IS ${TOR_BINARY}"

if "${TOR_BINARY}" --list-modules | grep -q "relay: no"; then
  echo "This test requires the relay module. Skipping." >&2
  exit 77
fi

tmpdir=
# For some reasons, shellcheck is not seeing that we can call this
# function from the trap below.
# shellcheck disable=SC2317
clean () {
  if [ -n "$tmpdir" ] && [ -d "$tmpdir" ]; then
    rm -rf "$tmpdir"
  fi
}

trap clean EXIT HUP INT TERM

tmpdir="$(mktemp -d -t tor_rebind_test.XXXXXX)"
if [ -z "$tmpdir" ]; then
  echo >&2 mktemp failed
  exit 2
elif [ ! -d "$tmpdir" ]; then
  echo >&2 mktemp failed to make a directory
  exit 3
fi

"${PYTHON:-python}" "${abs_top_srcdir:-.}/src/test/test_rebind.py" "${TOR_BINARY}" "$tmpdir"

exit $?
