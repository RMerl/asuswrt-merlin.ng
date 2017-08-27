#!/bin/sh
if [ ! "$1" ] ; then
	echo `basename $0` file ...
	echo '  convert' filenames from dos to unix
	exit 1
fi

while [ "$1" ] ; do
	TMP=$1.$$
	if tr -d '\r' <"$1" >"$TMP" ; then
		cp -a -f "$TMP" "$1"
	fi
	rm -f "$TMP"
	shift
done

