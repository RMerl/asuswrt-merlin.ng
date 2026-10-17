#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# Script to check whether the file exists in mkimage cfg files for the i.MX9.
#
# usage: $0 <file.cfg>

file=$1

blobs=`awk '/^APPEND/ {print $2} /^IMAGE/ || /^DATA/ {print $3}' $file`
for f in $blobs; do
	tmp=$srctree/$f
	if [ $f = "u-boot-spl-ddr.bin" ]; then
		continue
	fi

	if [ -f $f ]; then
		continue
	fi

	if [ ! -f $tmp ]; then
		echo "WARNING '$tmp' not found, resulting binary may be not-functional" >&2

                # Comment-out the lines for un-existing files. This way,
                # mkimage can keep working. This allows CI tests to pass even
                # if the resulting binary won't boot.
                sed -in "/$f/ s/./#&/" $file
	fi
done

exit 0
