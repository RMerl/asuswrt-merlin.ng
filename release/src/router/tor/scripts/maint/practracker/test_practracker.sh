#!/bin/sh

# Fail this script if any subprocess fails unexpectedly.
set -e

umask 077
unset TOR_DISABLE_PRACTRACKER

TMPDIR=""
clean() {
  if [ -n "$TMPDIR" ] && [ -d "$TMPDIR" ]; then
        rm -rf "$TMPDIR"
  fi
}
trap clean EXIT HUP INT TERM

if test "${PRACTRACKER_DIR}" = "" ||
        test ! -e "${PRACTRACKER_DIR}/practracker.py" ; then
    PRACTRACKER_DIR=$(dirname "$0")
fi

# Change to the tor directory, and canonicalise PRACTRACKER_DIR,
# so paths in practracker output are consistent, even in out-of-tree builds
cd "${PRACTRACKER_DIR}"/../../..
PRACTRACKER_DIR="scripts/maint/practracker"

TMPDIR="$(mktemp -d -t pracktracker.test.XXXXXX)"
if test -z "${TMPDIR}" || test ! -d "${TMPDIR}" ; then
    echo >&2 "mktemp failed."
    exit 1
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
        "${DATA}/" "$@" || echo "practracker exit status: $?"
}

compare() {
    # we can't use cmp because we need to use -b for windows
    diff -b -u "$@" > "${TMPDIR}/test-diff" || true
    if test -z "$(cat "${TMPDIR}"/test-diff)"; then
        echo "OK"
    else
        cat "${TMPDIR}/test-diff"
        echo "FAILED"
        exit 1
    fi
}

echo "unit tests:"

"${PYTHON:-python}" "${PRACTRACKER_DIR}/practracker_tests.py"

echo "ex0:"

run_practracker --exceptions "${DATA}/ex0.txt" \
                > "${TMPDIR}/ex0-received.txt" 2>&1

compare "${TMPDIR}/ex0-received.txt" \
        "${DATA}/ex0-expected.txt"

echo "ex1:"

run_practracker --exceptions "${DATA}/ex1.txt" \
                > "${TMPDIR}/ex1-received.txt" 2>&1

compare "${TMPDIR}/ex1-received.txt" \
        "${DATA}/ex1-expected.txt"

echo "ex1.overbroad:"

run_practracker --exceptions "${DATA}/ex1.txt" --list-overbroad \
                > "${TMPDIR}/ex1-overbroad-received.txt" 2>&1

compare "${TMPDIR}/ex1-overbroad-received.txt" \
        "${DATA}/ex1-overbroad-expected.txt"

echo "ex1.regen:"

cp "${DATA}/ex1.txt" "${TMPDIR}/ex1-copy.txt"
run_practracker --exceptions "${TMPDIR}/ex1-copy.txt" --regen >/dev/null 2>&1
compare "${TMPDIR}/ex1-copy.txt" "${DATA}/ex1-regen-expected.txt"

echo "ex1.regen_overbroad:"

cp "${DATA}/ex1.txt" "${TMPDIR}/ex1-copy.txt"
run_practracker --exceptions "${TMPDIR}/ex1-copy.txt" --regen-overbroad >/dev/null 2>&1
compare "${TMPDIR}/ex1-copy.txt" "${DATA}/ex1-regen-overbroad-expected.txt"
