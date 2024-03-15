#!/bin/sh
# Copyright 2019, The Tor Project, Inc.
# See LICENSE for licensing information

# Integration test script for verifying that Tor configurations are parsed as
# we expect.
#
# Valid configurations are tested with --dump-config, which parses and
# validates the configuration before writing it out.  We then make sure that
# the result is what we expect, before parsing and dumping it again to make
# sure that there is no change. Optionally, we can also test the log messages
# with --verify-config.
#
# Invalid configurations are tested with --verify-config, which parses
# and validates the configuration. We capture its output and make sure that
# it contains the error message we expect.
#
# When tor is compiled with different libraries or modules, some
# configurations may have different results. We can specify these result
# variants using additional result files.

# This script looks for its test cases as individual directories in
# src/test/conf_examples/.  Each test may have these files:
#
# Configuration Files
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
# (included torrc files or directories) -- Optional. Additional files can be
#      included in configuration, using the "%include" directive. Files or
#      directories can be included in any of the config files listed above.
#      Include paths should be specified relative to the test case directory.
#
# Result Files
#
# expected -- If this file is present, then it should be the expected result
#      of "--dump-config short" for this test case.  Exactly one of
#      "expected" or "error" must be present, or the test will fail.
#
# expected_log -- Optional. If this file is present, then it contains a regex
#      that must be matched by some line in the output of "--verify-config",
#      which must succeed. Only used if "expected" is also present.
#
# error -- If this file is present, then it contains a regex that must be
#      matched by some line in the output of "--verify-config", which must
#      fail. Exactly one of "expected" or "error" must be present, or the
#      test will fail.
#
# {expected,expected_log,error}_${TOR_LIBS_ENABLED}* -- If this file is
#      present, then the outcome is different when some optional libraries are
#      enabled. If there is no result file matching the exact list of enabled
#      libraries, the script searches for result files with one or more of
#      those libraries disabled. The search terminates at the standard result
#      file. If expected* is present, the script also searches for
#      expected_log*.
#
#      For example:
#      A test that succeeds, regardless of any enabled libraries:
#       - expected
#      A test that has a different result if the nss library is enabled
#      (but the same result if any other library is enabled). We also check
#      the log output in this test:
#       - expected
#       - expected_log
#       - expected_nss
#       - expected_log_nss
#      A test that fails if the lzma and zstd modules are *not* enabled:
#       - error
#       - expected_lzma_zstd
#
# {expected,expected_log,error}*_no_${TOR_MODULES_DISABLED} -- If this file is
#      present, then the outcome is different when some modules are disabled.
#      If there is no result file matching the exact list of disabled modules,
#      the standard result file is used. If expected* is present, the script
#      also searches for expected_log*.
#
#      For example:
#      A test that succeeds, regardless of any disabled modules:
#       - expected
#      A test that has a different result if the relay module is disabled
#      (but the same result if just the dirauth module is disabled):
#       - expected
#       - expected_no_relay_dirauth
#      A test that fails if the dirauth module is disabled:
#       - expected
#       - error_no_dirauth
#       - error_no_relay_dirauth
#      (Disabling the relay module also disables dirauth module. But we don't
#      want to encode that knowledge in this test script, so we supply a
#      separate result file for every combination of disabled modules that
#      has a different result.)
#
#      This logic ignores modules that are not listed by --list-modules
#      (dircache) and some that do not currently affect config parsing (pow).

umask 077
set -e

MYNAME="$0"

# emulate realpath(), in case coreutils or equivalent is not installed.
abspath() {
    f="$*"
    if test -d "$f"; then
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
if test $# -ge 1; then
  TOR_BINARY="$1"
  shift
else
  TOR_BINARY="${TESTING_TOR_BINARY:-./src/app/tor}"
fi

TOR_BINARY="$(abspath "$TOR_BINARY")"

echo "Using Tor binary '$TOR_BINARY'."

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

FINAL_EXIT=0
NEXT_TEST=

# Log a failure message to stderr, using $@ as a printf string and arguments
# Set NEXT_TEST to "yes" and FINAL_EXIT to $EXITCODE.
fail_printf()
{
    printf "FAIL: " >&2
    # The first argument is a printf string, so this warning is spurious
    # shellcheck disable=SC2059
    printf "$@" >&2
    NEXT_TEST="yes"
    FINAL_EXIT=$EXITCODE
}

# Log a failure message to stderr, using $@ as a printf string and arguments
# Exit with status $EXITCODE.
die_printf()
{
    printf "FAIL: CRITICAL error in '%s':" "$MYNAME" >&2
    # The first argument is a printf string, so this warning is spurious
    # shellcheck disable=SC2059
    printf "$@" >&2
    exit $EXITCODE
}

if test "$WINDOWS" = 1; then
    FILTER="dos2unix"
else
    FILTER="cat"
fi

EMPTY="${DATA_DIR}/EMPTY"
touch "$EMPTY" || die_printf "Couldn't create empty file '%s'.\\n" \
                             "$EMPTY"
NON_EMPTY="${DATA_DIR}/NON_EMPTY"
echo "This pattern should not match any log messages" \
     > "$NON_EMPTY" || die_printf "Couldn't create non-empty file '%s'.\\n" \
                                  "$NON_EMPTY"

STANDARD_LIBS="libevent\\|openssl\\|zlib"
MODULES_WITHOUT_CONFIG_TESTS="dircache\\|pow"

# Lib names are restricted to [a-z0-9]* at the moment
# We don't actually want to support foreign accents here
# shellcheck disable=SC2018,SC2019
TOR_LIBS_ENABLED="$("$TOR_BINARY" --verify-config \
                      -f "$EMPTY" --defaults-torrc "$EMPTY" \
                    | sed -n 's/.* Tor .* running on .* with\(.*\) and .* .* as libc\./\1/p' \
                    | tr 'A-Z' 'a-z' | tr ',' '\n' \
                    | grep -v "$STANDARD_LIBS" | grep -v "n/a" \
                    | sed 's/\( and\)* \(lib\)*\([a-z0-9]*\) .*/\3/' \
                    | sort | tr '\n' '_')"
# Remove the last underscore, if there is one
TOR_LIBS_ENABLED=${TOR_LIBS_ENABLED%_}

# If we ever have more than 3 optional libraries, we'll need more code here
TOR_LIBS_ENABLED_COUNT="$(echo "$TOR_LIBS_ENABLED_SEARCH" \
                          | tr ' ' '\n' | wc -l)"
if test "$TOR_LIBS_ENABLED_COUNT" -gt 3; then
    die_printf "Can not handle more than 3 optional libraries.\\n"
fi
# Brute-force the combinations of libraries
TOR_LIBS_ENABLED_SEARCH_3="$(echo "$TOR_LIBS_ENABLED" \
    | sed -n \
      's/^\([^_]*\)_\([^_]*\)_\([^_]*\)$/_\1_\2 _\1_\3 _\2_\3 _\1 _\2 _\3/p')"
TOR_LIBS_ENABLED_SEARCH_2="$(echo "$TOR_LIBS_ENABLED" \
    | sed -n 's/^\([^_]*\)_\([^_]*\)$/_\1 _\2/p')"
TOR_LIBS_ENABLED_SEARCH="_$TOR_LIBS_ENABLED \
                           $TOR_LIBS_ENABLED_SEARCH_3 \
                           $TOR_LIBS_ENABLED_SEARCH_2"
TOR_LIBS_ENABLED_SEARCH="$(echo "$TOR_LIBS_ENABLED_SEARCH" | tr ' ' '\n' \
                           | grep -v '^_*$' | tr '\n' ' ')"

TOR_MODULES_DISABLED="$("$TOR_BINARY" --list-modules | grep ': no' \
                        | grep -v "$MODULES_WITHOUT_CONFIG_TESTS" \
                        | cut -d ':' -f1 | sort | tr '\n' '_')"
# Remove the last underscore, if there is one
TOR_MODULES_DISABLED=${TOR_MODULES_DISABLED%_}

echo "Tor is configured with:"
echo "Optional Libraries: ${TOR_LIBS_ENABLED:-(None)}"
if test "$TOR_LIBS_ENABLED"; then
    echo "Optional Library Search List: $TOR_LIBS_ENABLED_SEARCH"
fi
echo "Disabled Modules: ${TOR_MODULES_DISABLED:-(None)}"

# Yes, unix uses "0" for a successful command
TRUE=0
FALSE=1

# Run tor --verify-config on the torrc $1, and defaults torrc $2, which may
# be $EMPTY. Pass tor the extra command line arguments $3, which will be
# passed unquoted.
# Send tor's standard output to stderr.
log_verify_config()
{
    # show the command we're about to execute
    # log_verify_config() is only called when we've failed
    printf "Tor --verify-config said:\\n" >&2
    printf "$ %s %s %s %s %s %s %s\\n" \
    "$TOR_BINARY" --verify-config \
                  -f "$1" \
                  --defaults-torrc "$2" \
                  "$3" \
                  >&2
    # We need cmdline unquoted
    # shellcheck disable=SC2086
    "$TOR_BINARY" --verify-config \
                  -f "$1" \
                  --defaults-torrc "$2" \
                  $3 \
                  >&2 \
        || true
}

# Run "tor --dump-config short" on the torrc $1, and defaults torrc $2, which
# may be $EMPTY. Pass tor the extra command line arguments $3, which will be
# passed unquoted. Send tor's standard output to $4.
#
# Set $FULL_TOR_CMD to the tor command line that was executed.
#
# If tor fails, fail_printf() using the file name $5, and context $6,
# which may be an empty string. Then run log_verify_config().
dump_config()
{
    if test "$6"; then
        CONTEXT=" $6"
    else
        CONTEXT=""
    fi

    # keep the command we're about to execute, and show if it we fail
    FULL_TOR_CMD=$(printf "$ %s %s %s %s %s %s %s %s" \
         "$TOR_BINARY" --dump-config short \
                       -f "$1" \
                       --defaults-torrc "$2" \
                       "$3"
             )
    # We need cmdline unquoted
    # shellcheck disable=SC2086
    if ! "$TOR_BINARY" --dump-config short \
                       -f "$1" \
                       --defaults-torrc "$2" \
                       $3 \
                       > "$4"; then
        fail_printf "'%s': Tor --dump-config reported an error%s:\\n%s\\n" \
                    "$5" \
                    "$CONTEXT" \
                    "$FULL_TOR_CMD"
        log_verify_config "$1" \
                          "$2" \
                          "$3"
    fi
}

# Run "$FILTER" on the input $1.
# Send the standard output to $2.
# If tor fails, log a failure message using the file name $3, and context $4,
# which may be an empty string.
filter()
{
    if test "$4"; then
        CONTEXT=" $4"
    else
        CONTEXT=""
    fi

    "$FILTER" "$1" \
              > "$2" \
        || fail_printf "'%s': Filter '%s' reported an error%s.\\n" \
                       "$3" \
                       "$FILTER" \
                       "$CONTEXT"
}

# Compare the expected file $1, and output file $2.
#
# If they are different, fail. Log the differences between the files.
# Run log_verify_config() with torrc $3, defaults torrc $4, and command
# line $5, to log Tor's error messages.
#
# If the file contents are identical, returns true. Otherwise, return false.
#
# Log failure messages using fail_printf(), with the expected file name,
# context $6, which may be an empty string, and the tor command line $7.
check_diff()
{
    if test "$6"; then
        CONTEXT=" $6"
    else
        CONTEXT=""
    fi

    if cmp "$1" "$2" > /dev/null; then
        return "$TRUE"
    else
        fail_printf "'%s': Tor --dump-config said%s:\\n%s\\n" \
                    "$1" \
                    "$CONTEXT" \
                    "$7"
        diff -u "$1" "$2" >&2 \
            || true
        log_verify_config "$3" \
                          "$4" \
                          "$5"
        return "$FALSE"
    fi
}

# Run "tor --dump-config short" on the torrc $1, and defaults torrc $2, which
# may be $EMPTY. Pass tor the extra command line arguments $3, which will be
# passed unquoted. Send tor's standard output to $4, after running $FILTER
# on it.
#
# If tor fails, run log_verify_config().
#
# Compare the expected file $5, and output file. If they are different, fail.
# If this is the first step that failed in this test, run log_verify_config().
#
# If the file contents are identical, returns true. Otherwise, return false,
# and log the differences between the files.
#
# Log failure messages using fail_printf(), with the expected file name, and
# context $6, which may be an empty string.
check_dump_config()
{
    OUTPUT="$4"
    OUTPUT_RAW="${OUTPUT}_raw"

    FULL_TOR_CMD=
    dump_config "$1" \
                "$2" \
                "$3" \
                "$OUTPUT_RAW" \
                "$5" \
                "$6"

    filter "$OUTPUT_RAW" \
           "$OUTPUT" \
           "$5" \
           "$6"

    if check_diff "$5" \
                  "$OUTPUT" \
                  "$1" \
                  "$2" \
                  "$3" \
                  "$6" \
                  "$FULL_TOR_CMD"; then
        return "$TRUE"
    else
        return "$FALSE"
    fi
}

# Check if $1 is an empty file.
# If it is, fail_printf() using $2 as the type of file.
# Returns true if the file is empty, false otherwise.
check_empty_pattern()
{
    if ! test -s "$1"; then
        fail_printf "%s file '%s' is empty, and will match any output.\\n" \
                    "$2" \
                    "$1"
        return "$TRUE"
    else
        return "$FALSE"
    fi
}

# Run tor --verify-config on the torrc $1, and defaults torrc $2, which may
# be $EMPTY. Pass tor the extra command line arguments $3, which will be
# passed unquoted. Send tor's standard output to $4.
#
# Set $FULL_TOR_CMD to the tor command line that was executed.
#
# If tor's exit status does not match the boolean $5, fail_printf()
# using the file name $6, and context $7, which is required.
verify_config()
{
    RESULT=$TRUE

    # keep the command we're about to execute, and show if it we fail
    FULL_TOR_CMD=$(printf "$ %s %s %s %s %s %s %s" \
    "$TOR_BINARY" --verify-config \
                  -f "$1" \
                  --defaults-torrc "$2" \
                  "$3"
             )
    # We need cmdline unquoted
    # shellcheck disable=SC2086
    "$TOR_BINARY" --verify-config \
                  -f "$1" \
                  --defaults-torrc "$2" \
                  $3 \
                  > "$4" || RESULT=$FALSE

    # Convert the actual and expected results to boolean, and compare
    if test $((! (! RESULT))) -ne $((! (! $5))); then
        fail_printf "'%s': Tor --verify-config did not %s:\\n%s\\n" \
                    "$6" \
                    "$7" \
                    "$FULL_TOR_CMD"
        cat "$4" >&2
    fi
}

# Check for the patterns in the match file $1, in the output file $2.
# Uses grep with the entire contents of the match file as the pattern.
# (Not "grep -f".)
#
# If the pattern does not match any lines in the output file, fail.
# Log the pattern, and the entire contents of the output file.
#
# Log failure messages using fail_printf(), with the match file name,
# context $3, and tor command line $4, which are required.
check_pattern()
{
    expect_log="$(cat "$1")"
    if ! grep "$expect_log" "$2" > /dev/null; then
        fail_printf "Expected %s '%s':\\n%s\\n" \
                    "$3" \
                    "$1" \
                    "$expect_log"
        printf "Tor --verify-config said:\\n%s\\n" \
               "$4" >&2
        cat "$2" >&2
    fi
}

# Run tor --verify-config on the torrc $1, and defaults torrc $2, which may
# be $EMPTY. Pass tor the extra command line arguments $3, which will be
# passed unquoted. Send tor's standard output to $4.
#
# If tor's exit status does not match the boolean $5, fail.
#
# Check for the patterns in the match file $6, in the output file.
# Uses grep with the entire contents of the match file as the pattern.
# (Not "grep -f".) The match file must not be empty.
#
# If the pattern does not match any lines in the output file, fail.
# Log the pattern, and the entire contents of the output file.
#
# Log failure messages using fail_printf(), with the match file name,
# and context $7, which is required.
check_verify_config()
{
    if check_empty_pattern "$6" "$7"; then
        return
    fi

    FULL_TOR_CMD=
    verify_config "$1" \
                  "$2" \
                  "$3" \
                  "$4" \
                  "$5" \
                  "$6" \
                  "$7"

    check_pattern "$6" \
                  "$4" \
                  "$7" \
                  "$FULL_TOR_CMD"
}

for dir in "${EXAMPLEDIR}"/*; do
    NEXT_TEST=

    if ! test -d "$dir"; then
       # Only count directories.
       continue
    fi

    testname="$(basename "${dir}")"
    # We use printf since "echo -n" is not standard
    printf "%s: " \
           "$testname"

    PREV_DIR="$(pwd)"
    cd "$dir"

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

    EXPECTED=
    EXPECTED_LOG=
    ERROR=
    # Search for a custom result file for any combination of enabled optional
    # libraries
    # The libs in the list are [A-Za-z0-9_]* and space-separated.
    # shellcheck disable=SC2086
    for lib_suffix in $TOR_LIBS_ENABLED_SEARCH ""; do
        # Search for a custom result file for any disabled modules
        for mod_suffix in "_no_${TOR_MODULES_DISABLED}" ""; do
            suffix="${lib_suffix}${mod_suffix}"

            if test -f "./expected${suffix}"; then

                # Check for broken configs
                if test -f "./error${suffix}"; then
                    fail_printf "Found both '%s' and '%s'.%s\\n" \
                                "${dir}/expected${suffix}" \
                                "${dir}/error${suffix}" \
                                "(Only one of these files should exist.)"
                    break
                fi

                EXPECTED="./expected${suffix}"
                if test -f "./expected_log${suffix}"; then
                    EXPECTED_LOG="./expected_log${suffix}"
                fi
                break

            elif test -f "./error${suffix}"; then
                ERROR="./error${suffix}"
                break
            fi
        done

        # Exit as soon as the inner loop finds a file, or fails
        if test -f "$EXPECTED" || test -f "$ERROR" || test "$NEXT_TEST"; then
            break
        fi
    done

    if test "$NEXT_TEST"; then
        # The test failed inside the file search loop: go to the next test
        continue
    elif test -f "$EXPECTED"; then
        # This case should succeed: run dump-config and see if it does.

        if check_dump_config "./torrc" \
                             "$DEFAULTS" \
                             "$CMDLINE" \
                             "${DATA_DIR}/output.${testname}" \
                             "$EXPECTED" \
                             ""; then
            # Check round-trip.
            check_dump_config "${DATA_DIR}/output.${testname}" \
                              "$EMPTY" \
                              "" \
                              "${DATA_DIR}/output_2.${testname}" \
                              "$EXPECTED" \
                              "on round-trip" || true
        fi

        if test -f "$EXPECTED_LOG"; then
            # This case should succeed: run verify-config and see if it does.

            check_verify_config "./torrc" \
                                "$DEFAULTS" \
                                "$CMDLINE" \
                                "${DATA_DIR}/output_log.${testname}" \
                                "$TRUE" \
                                "$EXPECTED_LOG" \
                                "log success"
        else
            printf "\\nNOTICE: Missing '%s_log' file:\\n" \
                   "$EXPECTED" >&2
            log_verify_config "./torrc" \
                              "$DEFAULTS" \
                              "$CMDLINE"
        fi

   elif test -f "$ERROR"; then
        # This case should fail: run verify-config and see if it does.

        check_verify_config "./torrc" \
                            "$DEFAULTS" \
                            "$CMDLINE" \
                            "${DATA_DIR}/output.${testname}" \
                            "$FALSE" \
                            "$ERROR" \
                            "log error"
    else
        # This case is not actually configured with a success or a failure.
        # call that an error.
        fail_printf "Did not find ${dir}/*expected or ${dir}/*error.\\n"
    fi

    if test -z "$NEXT_TEST"; then
        echo "OK"
    fi

    cd "$PREV_DIR"

done

exit "$FINAL_EXIT"
