#!/bin/bash

DIR=$(dirname `readlink -f $0`)
. $DIR/testing.conf

rm -f $LOGFILE
mkdir -p $BUILDDIR

if [ $ENABLE_BUILD_BASEIMAGE = "yes" ]
then
	$DIR/scripts/build-baseimage || exit 1
fi

if [ $ENABLE_BUILD_ROOTIMAGE = "yes" ]
then
	$DIR/scripts/build-rootimage || exit 1
fi

if [ $ENABLE_BUILD_GUESTKERNEL = "yes" ]
then
	$DIR/scripts/build-guestkernel || exit 1
fi

if [ $ENABLE_BUILD_GUESTIMAGES = "yes" ]
then
	$DIR/scripts/build-guestimages $HOSTS || exit 1
fi
