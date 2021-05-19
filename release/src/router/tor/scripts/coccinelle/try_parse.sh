#!/bin/sh

# Echo the name of every argument of this script that is not "perfect"
# according to coccinelle's --parse-c.
#
# If $TOR_COCCI_EXCEPTIONS_FILE is non-empty, skip any files that match the
# patterns in the exception file, according to "grep -f"
#
# If VERBOSE is non-empty, log spatch errors and skipped files.

top="$(dirname "$0")/../.."

exitcode=0

for fn in "$@"; do

    if test "${TOR_COCCI_EXCEPTIONS_FILE}" ; then
        skip_fn=$(echo "$fn" | grep -f "${TOR_COCCI_EXCEPTIONS_FILE}")
        if test "${skip_fn}" ; then
            if test "${VERBOSE}" != ""; then
                echo "Skipping '${skip_fn}'"
            fi
            continue
        fi
    fi

    if spatch --macro-file-builtins \
              "$top"/scripts/coccinelle/tor-coccinelle.h \
              --defined COCCI \
              --parse-c "$fn" \
              2>/dev/null | grep "perfect = 1" > /dev/null; then
        : # it's perfect
    else
        echo "$fn"
        if test "${VERBOSE}" != ""; then
            spatch --macro-file-builtins \
                   "$top"/scripts/coccinelle/tor-coccinelle.h \
                   --defined COCCI \
                   --parse-c "$fn"
        fi
        exitcode=1
    fi

done

exit "$exitcode"
