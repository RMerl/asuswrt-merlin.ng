#!/bin/sh -e

parentdir=`dirname $0`

cd $parentdir
parentdir=`pwd`

libtoolize -f -c
#aclocal
autoheader
autoconf

mysubdirs="$mysubdirs `find src/modules/ -name configure -print | sed 's%/configure%%'`"
mysubdirs=`echo $mysubdirs`

for F in $mysubdirs
do
	echo "Configuring in $F..."
	(cd $F && grep "^AC_CONFIG_HEADER" configure.ac > /dev/null || exit 0; autoheader -I$parentdir)
	(cd $F && autoconf -I$parentdir)
done
