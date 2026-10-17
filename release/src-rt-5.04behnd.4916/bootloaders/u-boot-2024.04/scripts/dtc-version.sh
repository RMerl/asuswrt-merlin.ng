#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# dtc-version dtc-command
#
# Prints the dtc version of `dtc-command' in a canonical 6-digit form
# such as `010404'  for dtc 1.4.4
#

dtc="$*"

if [ ${#dtc} -eq 0 ]; then
	echo "Error: No dtc command specified"
	printf "Usage:\n\t$0 <dtc-command>\n"
	exit 1
fi

if ! which $dtc >/dev/null ; then
	echo "Error: Cannot find dtc: $dtc"
	exit 1
fi

MAJOR=$($dtc -v | head -1 | awk '{print $NF}' | cut -d . -f 1 | tr -d v)
MINOR=$($dtc -v | head -1 | awk '{print $NF}' | cut -d . -f 2)
PATCH=$($dtc -v | head -1 | awk '{print $NF}' | cut -d . -f 3 | cut -d - -f 1)

printf "%02d%02d%02d\\n" $MAJOR $MINOR $PATCH
