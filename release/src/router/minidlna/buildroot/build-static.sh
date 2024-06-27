#!/bin/bash -e
if [ $# -ne 1 ]; then
	echo "Usage: $0 <BUILDROOT_PATH>"
	exit 1
fi
BUILDROOT_DIR=$1
BR2_DEFCONFIG=$(realpath readymedia_defconfig)
export BR2_EXTERNAL=$(realpath .)
cd $BUILDROOT_DIR
make O=output-readymedia defconfig BR2_DEFCONFIG=${BR2_DEFCONFIG}
make O=output-readymedia
echo -e "\n\nStatic binary built in $(realpath output-readymedia/target/usr/sbin/minidlnad)"
ls -lh $(realpath output-readymedia/target/usr/sbin/minidlnad)
