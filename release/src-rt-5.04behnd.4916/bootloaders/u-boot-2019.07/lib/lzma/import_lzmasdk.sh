#!/bin/sh

usage() {
	echo "Usage: $0 lzmaVERSION.tar.bz2" >&2
	echo >&2
	exit 1
}

if [ "$1" = "" ] ; then
	 usage
fi

if [ ! -f $1 ] ; then
	echo "$1 doesn't exist!" >&2
	exit 1
fi

BASENAME=`basename $1 .tar.bz2`
TMPDIR=/tmp/tmp_lib_$BASENAME
FILES="C/LzmaDec.h
      C/Types.h
      C/LzmaDec.c
      history.txt
      lzma.txt"

mkdir -p $TMPDIR
echo "Untar $1 -> $TMPDIR"
tar -jxf $1 -C $TMPDIR

for i in $FILES; do
	echo Copying  $TMPDIR/$i \-\> `basename $i`
	cp $TMPDIR/$i .
	chmod -x `basename $i`
done

echo "done!"
