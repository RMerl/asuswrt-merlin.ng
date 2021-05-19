#!/bin/sh
# Check that tor regenerates keys when key files are zero-length

umask 077
set -e

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

# find the tor binary
if [ $# -ge 1 ]; then
  TOR_BINARY="${1}"
  shift
else
  TOR_BINARY="${TESTING_TOR_BINARY:-./src/app/tor}"
fi

TOR_BINARY="$(abspath "$TOR_BINARY")"

echo "TOR BINARY IS ${TOR_BINARY}"

if "$TOR_BINARY" --list-modules | grep -q "relay: no"; then
  echo "This test requires the relay module. Skipping." >&2
  exit 77
fi

exitcode=0

"${SHELL:-sh}" "${abs_top_srcdir:-.}/src/test/zero_length_keys.sh" "$TOR_BINARY" -z || exitcode=1
"${SHELL:-sh}" "${abs_top_srcdir:-.}/src/test/zero_length_keys.sh" "$TOR_BINARY" -d || exitcode=1
"${SHELL:-sh}" "${abs_top_srcdir:-.}/src/test/zero_length_keys.sh" "$TOR_BINARY" -e || exitcode=1

exit ${exitcode}
