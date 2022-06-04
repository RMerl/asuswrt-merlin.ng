#!/bin/bash

DIR=`mktemp -d`
pushd ../.. > /dev/null
git archive --format=tar --prefix=hostap-build/ HEAD > $DIR/hostap-build.tar
popd > /dev/null

echo "Build test directory: $DIR"
echo

for i in build-hostapd-*.config; do
    ./build-hostapd.sh $DIR $i
done

for i in build-wpa_supplicant-*.config; do
    ./build-wpa_supplicant.sh $DIR $i
done

echo
echo "Build test directory: $DIR"
