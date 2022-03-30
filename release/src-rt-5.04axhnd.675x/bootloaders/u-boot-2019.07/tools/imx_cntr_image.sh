#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# script to check whether the file exists in imximage.cfg for i.MX8
#
# usage: $0 <imximage.cfg>

file=$1

blobs=`awk '/^APPEND/ {print $2} /^IMAGE/ || /^DATA/ {print $3}' $file`
for f in $blobs; do
	tmp=$srctree/$f
	if [ $f = "u-boot-dtb.bin" ]; then
		continue
	fi

	if [ -f $f ]; then
		continue
	fi

	if [ ! -f $tmp ]; then
		echo "WARNING '$tmp' not found, resulting binary is not-functional" >&2
		exit 1
	fi

	sed -in "s;$f;$tmp;" $file
done

exit 0
