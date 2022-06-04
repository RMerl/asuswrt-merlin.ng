#!/bin/bash

# Usage: ./runFormatlinux.sh dm-file-name
#
# Note!! This script needs to know the root of your build directory.
# You can set $BUILD_ROOT as an environment variable,
# or you can hard code it in the line below.
#
# BUILD_ROOT=/your/hardcoded/build_root
#
# Or you can let the script figure out your build directory.
# For this last option, this script must be run from either the
# hostTools/DataModelDesigner or the data-model directory.
#
if [[ -z "$BUILD_ROOT" ]]; then
#	echo "BUILD_ROOT not set, guess based on current directory..."
	REAL_PWD=`pwd`
	BUILD_ROOT=`pwd | sed -e 's/hostTools\/DataModelDesigner//'`
	if [[ $REAL_PWD == $BUILD_ROOT ]]; then
		# last sed had no effect, so maybe we are in data-model dir
		BUILD_ROOT=`pwd | sed -e 's/data-model//'`
	fi
#	echo "guessing BUILD_ROOT = $BUILD_ROOT"
fi



if [ -z "$1" ]; then
echo "Error: must specify data model filename as first argument."
exit 1
else
dmf=$1
fi

java -jar $BUILD_ROOT/hostTools/DataModelDesigner/DataModelDesigner.jar -quiet -brcm_dev -formatOnly -buildroot $BUILD_ROOT -dmfile $dmf


