#!/bin/bash

DIR=$1
CONF=$2
if [ -z "$DIR" -o -z "$CONF" ]; then
    echo "usage: $0 <DIR> <CONF>"
    exit 1
fi
if [ ! -d "$DIR" ]; then
   echo "DIR does not exist: $DIR"
   exit 1
fi
if [ ! -r "$CONF" ]; then
   echo "CONF does not exist: $CONF"
   exit 1
fi

NAME=`echo $CONF | sed s/^build-wpa_supplicant-// | sed s/\.config$//`
echo -n "wpa_supplicant build: $NAME - "

pushd $DIR > /dev/null
rm -rf hostap-build
tar xf hostap-build.tar
popd > /dev/null
cp $CONF $DIR/hostap-build/wpa_supplicant/.config

pushd $DIR/hostap-build/wpa_supplicant > /dev/null
if make -j8 > $DIR/wpa_supplicant-$NAME.log 2>&1; then
    mv $DIR/wpa_supplicant-$NAME.log{,-OK}
    echo OK
else
    mv $DIR/wpa_supplicant-$NAME.log{,-FAIL}
    echo FAIL
fi
rm -rf $DIR/hostap-build
popd > /dev/null
