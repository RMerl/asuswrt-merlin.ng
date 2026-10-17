#!/bin/sh
#

record_bcm_swversion () {
	IMAGE_VERSION=${BRCM_VERSION}.${BRCM_RELEASE}.${BRCM_EXTRAVERSION}
	echo $IMAGE_VERSION > $ROOTFS/etc/bcmwrt_swversion
}


