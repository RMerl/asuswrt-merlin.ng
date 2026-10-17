#!/bin/sh
# $1 - driver release zip file
# $2 - linux SDK root path (current dir if omitted)

DRV_REL=$1

if [ -n "$2" ]; then
  SDK_ROOT=$2
else
  SDK_ROOT=.
fi

XLIST=

if ! test -f $SDK_ROOT/bcmdrivers/opensource/include/bcm963xx/DiagDef.h; then
  XLIST=bcmdrivers/opensource/include/bcm963xx/DiagDef.h
fi
if ! test -f $SDK_ROOT/bcmdrivers/broadcom/include/bcm963xx/DiagDef.h; then
  XLIST="$XLIST bcmdrivers/broadcom/include/bcm963xx/DiagDef.h"
fi

#echo $XLIST
touch `unzip -o $DRV_REL -x $XLIST | awk '{print $2}'`
