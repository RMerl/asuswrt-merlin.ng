#!/bin/sh
#
# binutils-version [-p] gas-command
#
# Prints the binutils version of `gas-command' in a canonical 4-digit form
# such as `0222' for binutils 2.22
#

gas="$*"

if [ ${#gas} -eq 0 ]; then
	echo "Error: No assembler specified."
	printf "Usage:\n\t$0 <gas-command>\n"
	exit 1
fi

version_string=$($gas --version | head -1 | \
	sed -e 's/(.*)//; s/[^0-9.]*\([0-9.]*\).*/\1/')

MAJOR=$(echo $version_string | cut -d . -f 1)
MINOR=$(echo $version_string | cut -d . -f 2)

printf "%02d%02d\\n" $MAJOR $MINOR
