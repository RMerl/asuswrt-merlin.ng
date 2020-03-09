#!/bin/sh

umask 077
unset TOR_DISABLE_PRACTRACKER

TMPDIR=""
clean () {
  if [ -n "$TMPDIR" ] && [ -d "$TMPDIR" ]; then
    rm -rf "$TMPDIR"
  fi
}
trap clean EXIT HUP INT TERM

if test "${PRACTRACKER_DIR}" = "" ||
        test ! -e "${PRACTRACKER_DIR}/practracker.py" ; then
    PRACTRACKER_DIR=$(dirname "$0")
fi

TMPDIR="$(mktemp -d -t pracktracker.test.XXXXXX)"
if test -z "${TMPDIR}" || test ! -d "${TMPDIR}" ; then
    echo >&2 "mktemp failed."
    exit 1;
fi

DATA="${PRACTRACKER_DIR}/testdata"

run_practracker() {
    "${PYTHON:-python}" "${PRACTRACKER_DIR}/practracker.py" \
        --include-dir "" \
        --max-file-size=0 \
        --max-function-size=0 \
        --max-h-file-size=0 \
        --max-h-include-count=0 \
        --max-include-count=0 \
        --terse \
        "${DATA}/" "$@";
}
compare() {
    # we can't use cmp because we need to use -b for windows
    diff -b -u "$@" > "${TMPDIR}/test-diff"
    if test -z "$(cat "${TMPDIR}"/test-diff)"; then
        echo "OK"
    else
        cat "${TMPDIR}/test-diff"
        echo "FAILED"
        exit 1
    fi
}

echo "unit tests:"

"${PYTHON:-python}" "${PRACTRACKER_DIR}/practracker_tests.py" || exit 1

echo "ex0:"

run_practracker --exceptions "${DATA}/ex0.txt" > "${TMPDIR}/ex0-received.txt"

compare "${TMPDIR}/ex0-received.txt" "${DATA}/ex0-expected.txt"

echo "ex1:"

run_practracker --exceptions "${DATA}/ex1.txt" > "${TMPDIR}/ex1-received.txt"

compare "${TMPDIR}/ex1-received.txt" "${DATA}/ex1-expected.txt"

echo "ex1.overbroad:"

run_practracker --exceptions "${DATA}/ex1.txt" --list-overbroad > "${TMPDIR}/ex1-overbroad-received.txt"

compare "${TMPDIR}/ex1-overbroad-received.txt" "${DATA}/ex1-overbroad-expected.txt"
