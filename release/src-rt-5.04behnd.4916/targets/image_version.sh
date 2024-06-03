#!/bin/bash

record_image_version () {
	IMAGE_VERSION=${BRCM_VERSION}${BRCM_RELEASE}
	IMAGE_VERSION+=$(echo ${BRCM_EXTRAVERSION} | sed -e "s/\(0\)\([1-9]\)/\2/")
	IMAGE_VERSION+=$(echo ${PROFILE} | sed -e "s/^[0-9]*//")$(date '+%j%H%M')
	echo $IMAGE_VERSION > $ROOTFS/rom/etc/image_version
}
