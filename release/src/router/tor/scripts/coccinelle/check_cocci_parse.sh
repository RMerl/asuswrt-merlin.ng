#!/bin/sh

# If we have coccinelle installed, run try_parse.sh on every filename passed
# as an argument. If no filenames are supplied, scan a standard Tor 0.3.5 or
# later directory layout.
#
# Uses the default coccinelle exceptions file, or $TOR_COCCI_EXCEPTIONS_FILE,
# if it is set.
#
# Use TOR_COCCI_EXCEPTIONS_FILE=/dev/null check_cocci_parse.sh to disable
# the default exception file.
#
# If spatch is not installed, remind the user to install it, but exit with
# a success error status.

scripts_cocci="$(dirname "$0")"
top="$scripts_cocci/../.."
try_parse="$scripts_cocci/try_parse.sh"

exitcode=0

export TOR_COCCI_EXCEPTIONS_FILE="${TOR_COCCI_EXCEPTIONS_FILE:-$scripts_cocci/exceptions.txt}"

PURPOSE="cocci C parsing"

echo "Checking spatch:"

if ! command -v spatch ; then
    echo "Install coccinelle's spatch to check $PURPOSE."
    exit "$exitcode"
fi

# Returns true if $1 is greater than or equal to $2
version_ge()
{
    if test "$1" = "$2" ; then
        # return true
        return 0
    fi
    LOWER_VERSION="$(printf '%s\n' "$1" "$2" | $SORT_V | head -n 1)"
    # implicit return
    test "$LOWER_VERSION" != "$1"
}

# 'sort -V' is a gnu extension
SORT_V="sort -V"
# Use 'sort -n' if 'sort -V' doesn't work
if ! version_ge "1" "0" ; then
    echo "Your 'sort -V' command appears broken. Falling back to 'sort -n'."
    echo "Some spatch version checks may give the wrong result."
    SORT_V="sort -n"
fi

# Print the full spatch version, for diagnostics
spatch --version

MIN_SPATCH_V="1.0.4"
# This pattern needs to handle version strings like:
# spatch version 1.0.0-rc19
# spatch version 1.0.6 compiled with OCaml version 4.05.0
SPATCH_V=$(spatch --version | head -1 | \
               sed 's/spatch version \([0-9][^ ]*\).*/\1/')

if ! version_ge "$SPATCH_V" "$MIN_SPATCH_V" ; then
    echo "Tor requires coccinelle spatch >= $MIN_SPATCH_V to check $PURPOSE."
    echo "But you have $SPATCH_V. Please install a newer version."
    exit "$exitcode"
fi

if test $# -ge 1 ; then
  "$try_parse" "$@"
  exitcode=$?
else
  cd "$top" || exit 1
  # This is the layout in 0.3.5
  # Keep these lists consistent:
  #   - OWNED_TOR_C_FILES in Makefile.am
  #   - CHECK_FILES in pre-commit.git-hook and pre-push.git-hook
  #   - try_parse in check_cocci_parse.sh
  "$try_parse" \
    src/lib/*/*.[ch] \
    src/core/*/*.[ch] \
    src/feature/*/*.[ch] \
    src/app/*/*.[ch] \
    src/test/*.[ch] \
    src/test/*/*.[ch] \
    src/tools/*.[ch]
  exitcode=$?
fi

if test "$exitcode" != 0 ; then
    echo "Please fix these $PURPOSE errors in the above files"
    echo "Set VERBOSE=1 for more details"
    echo "Try running test-operator-cleanup or 'make autostyle-operators'"
    echo "As a last resort, you can modify scripts/coccinelle/exceptions.txt"
fi

exit "$exitcode"
