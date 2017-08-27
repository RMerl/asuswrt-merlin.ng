#!/bin/sh
# $Id: fix.sh 1459 2009-08-18 04:28:45Z robert $

set -e 

if [ "X$1" = "Xsysstat" ] ; then
	FFILES="usr/lib/sysstat/sa[12]
		usr/share/man/man*/*
		usr/share/doc/sysstat/FAQ
		usr/share/doc/sysstat/examples/*"
elif [ "X$1" = "Xisag" ] ; then
	FFILES="usr/share/man/man*/*" 
else
	echo "Usage: $0 [ sysstat | isag ]" 1>&2
	exit 1
fi

dir="debian/$1"

if [ ! -d "$dir" ] ; then
	echo "Directory $dir does not exist!" 1>&2
	exit 1
fi

cd "$dir"

for file in `echo $FFILES`; do
 man_re=""
 # try to fix hyphens in systat's man pages
 if [ "${file%man/man*}" != "$file" ] && [ "${file%isag*}" = "$file" ]; then
 	man_re='2,${:S;s|\([^\\]\)-|\1\\-|g;tS}'
 fi	

 if [ -n "$man_re" ] || grep -q 'l[oi][gb]/sa' "$file" >/dev/null 2>&1 ; then
	echo  " + processing file: $dir/$file"
	mv "$file" _tmp_
	sed -e 's|usr/lib/sa|usr/lib/sysstat|g' \
	    -e 's|var/log/sa|var/log/sysstat|g' \
	    -e 's|usr/local/lib/sa|usr/local/lib/sysstat|g' \
	    -e 's|^\.IX|.\\"&|' \
	    -e "$man_re" \
		< _tmp_ > "$file"
	chmod --reference=_tmp_ "$file" 
	touch -r _tmp_	"$file"
	rm -f _tmp_
 fi
done

exit 0

