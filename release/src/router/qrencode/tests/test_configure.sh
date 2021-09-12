#!/bin/sh

BASEDIR=..

CONFIG_H_IN="$BASEDIR/config.h.in"
CONFIG_H="$BASEDIR/config.h"
LIBQRENCODE_PC_IN="$BASEDIR/libqrencode.pc.in"
LIBQRENCODE_PC="$BASEDIR/libqrencode.pc"

echo "Testing configure scripts..."

(cd $BASEDIR; ./autogen.sh)

# test config.h.in
grep "#undef HAVE_LIBPTHREAD" $CONFIG_H_IN > /dev/null
if test ! $? -eq 0; then
	echo "HAVE_LIBPTHREAD undefined in config.h.in."
	exit 1
fi

# test libqrencode.pc.in
grep "Libs.private: @LIBPTHREAD@" $LIBQRENCODE_PC_IN > /dev/null
if test ! $? -eq 0; then
	echo "Pthread is not handled in libqrencode.pc.in."
	exit 1
fi

# test pthread checks in configure
(cd $BASEDIR; ./configure --with-tests --enable-thread-safety > /dev/null)
grep "#define HAVE_LIBPTHREAD 1" $CONFIG_H > /dev/null
if test ! $? -eq 0; then
	echo "HAVE_LIBPTHREAD undefined in config.h."
	exit 1
fi

grep "Libs.private: -lpthread" $LIBQRENCODE_PC > /dev/null
if test ! $? -eq 0; then
	echo "Pthread is not handled in libqrencode.pc."
	exit 1
fi

(cd $BASEDIR; ./configure --with-tests --disable-thread-safety > /dev/null)
grep "#define HAVE_LIBPTHREAD 1" $CONFIG_H > /dev/null
if test ! $? -eq 1; then
	echo "HAVE_LIBPTHREAD incorrectly defined in config.h."
	exit 1
fi

grep "Libs.private: -lpthread" $LIBQRENCODE_PC > /dev/null
if test ! $? -eq 1; then
	echo "Pthread is incorrectly handled in libqrencode.pc."
	exit 1
fi

echo "All tests of configure script passed. Now reconfiguring..."

(cd $BASEDIR; ./configure --with-tests > /dev/null)

echo "Done."

exit 0
