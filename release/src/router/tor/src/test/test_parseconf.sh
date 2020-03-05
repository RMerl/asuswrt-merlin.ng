#!/bin/sh
# Copyright 2019, The Tor Project, Inc.
# See LICENSE for licensing information

# Integration test script for verifying that Tor configurations are parsed as
# we expect.
#
# Valid configurations are tested with --dump-config, which parses and
# validates the configuration before writing it out.  We then make sure that
# the result is what we expect, before parsing and dumping it again to make
# sure that there is no change.
#
# Invalid configurations are tested with --verify-config, which parses
# and validates the configuration. We capture its output and make sure that
# it contains the error message we expect.

# This script looks for its test cases as individual directories in
# src/test/conf_examples/.  Each test may have these files:
#
# torrc -- Usually needed. This file is passed to Tor on the command line
#      with the "-f" flag. (If you omit it, you'll test Tor's behavior when
#      it receives a nonexistent configuration file.)
#
# torrc.defaults -- Optional. If present, it is passed to Tor on the command
#      line with the --defaults-torrc option. If this file is absent, an empty
#      file is passed instead to prevent Tor from reading the system defaults.
#
# cmdline -- Optional. If present, it contains command-line arguments that
#      will be passed to Tor.
#
# expected -- If this file is present, then it should be the expected result
#      of "--dump-config short" for this test case.  Exactly one of
#      "expected" or "error" must be present, or the test will fail.
#
# error -- If this file is present, then it contains a regex that must be
#      matched by some line in the output of "--verify-config", which must
#      fail. Exactly one of "expected" or "error" must be present, or the
#      test will fail.

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

# make a safe space for temporary files
DATA_DIR=$(mktemp -d -t tor_parseconf_tests.XXXXXX)
trap 'rm -rf "$DATA_DIR"' 0

# This is where we look for examples
EXAMPLEDIR="$(dirname "$0")"/conf_examples

case "$(uname -s)" in
    CYGWIN*) WINDOWS=1;;
    MINGW*) WINDOWS=1;;
    MSYS*) WINDOWS=1;;
    *) WINDOWS=0;;
esac

####
# BUG WORKAROUND FOR 31757:
#  On Appveyor, it seems that Tor sometimes randomly fails to produce
#  output with --dump-config.  Whil we are figuring this out, do not treat
#  windows errors as hard failures.
####
if test "$WINDOWS" = 1; then
    EXITCODE=0
else
    EXITCODE=1
fi

die() { echo "$1" >&2 ; exit "$EXITCODE"; }

if test "$WINDOWS" = 1; then
    FILTER="dos2unix"
else
    FILTER="cat"
fi

touch "${DATA_DIR}/EMPTY" || die "Couldn't create empty file."

for dir in "${EXAMPLEDIR}"/*; do
    if ! test -d "${dir}"; then
       # Only count directories.
       continue
    fi

    testname="$(basename "${dir}")"
    # We use printf since "echo -n" is not standard
    printf "%s: " "$testname"

    PREV_DIR="$(pwd)"
    cd "${dir}"

    if test -f "./torrc.defaults"; then
        DEFAULTS="./torrc.defaults"
    else
        DEFAULTS="${DATA_DIR}/EMPTY"
    fi

    if test -f "./cmdline"; then
        CMDLINE="$(cat ./cmdline)"
    else
        CMDLINE=""
    fi

    if test -f "./expected"; then
        if test -f "./error"; then
            echo "FAIL: Found both ${dir}/expected and ${dir}/error."
            echo "(Only one of these files should exist.)"
            exit $EXITCODE
        fi

        # This case should succeed: run dump-config and see if it does.

        "${TOR_BINARY}" -f "./torrc" \
                        --defaults-torrc "${DEFAULTS}" \
                        --dump-config short \
                        ${CMDLINE} \
                        | "${FILTER}" > "${DATA_DIR}/output.${testname}" \
                        || die "Failure: Tor exited."

        if cmp "./expected" "${DATA_DIR}/output.${testname}">/dev/null ; then
            # Check round-trip.
            "${TOR_BINARY}" -f "${DATA_DIR}/output.${testname}" \
                            --defaults-torrc "${DATA_DIR}/empty" \
                            --dump-config short \
                            | "${FILTER}" \
                            > "${DATA_DIR}/output_2.${testname}" \
                        || die "Failure: Tor exited on round-trip."

            if ! cmp "${DATA_DIR}/output.${testname}" \
                 "${DATA_DIR}/output_2.${testname}"; then
                echo "Failure: did not match on round-trip."
                exit $EXITCODE
            fi

            echo "OK"
        else
            echo "FAIL"
            if test "$(wc -c < "${DATA_DIR}/output.${testname}")" = 0; then
                # There was no output -- probably we failed.
                "${TOR_BINARY}" -f "./torrc" \
                                --defaults-torrc "${DEFAULTS}" \
                                --verify-config \
                                ${CMDLINE} || true
            fi
            diff -u "./expected" "${DATA_DIR}/output.${testname}" || /bin/true
            exit $EXITCODE
        fi

   elif test -f "./error"; then
        # This case should fail: run verify-config and see if it does.

        "${TOR_BINARY}" --verify-config \
                        -f ./torrc \
                        --defaults-torrc "${DEFAULTS}" \
                        ${CMDLINE} \
                        > "${DATA_DIR}/output.${testname}" \
                        && die "Failure: Tor did not report an error."

        expect_err="$(cat ./error)"
        if grep "${expect_err}" "${DATA_DIR}/output.${testname}" >/dev/null; then
            echo "OK"
        else
            echo "FAIL"
            echo "Expected error: ${expect_err}"
            echo "Tor said:"
            cat "${DATA_DIR}/output.${testname}"
            exit $EXITCODE
        fi

    else
        # This case is not actually configured with a success or a failure.
        # call that an error.

        echo "FAIL: Did not find ${dir}/expected or ${dir}/error."
        exit $EXITCODE
    fi

    cd "${PREV_DIR}"

done
