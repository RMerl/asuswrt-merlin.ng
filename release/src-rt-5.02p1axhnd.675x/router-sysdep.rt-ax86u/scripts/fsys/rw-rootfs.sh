#!/bin/sh

if grep -q rootfs_ubifs /proc/mtd ; # grab line with "rootfs_ubifs"
then # UBIFS partition "rootfs_ubifs" was found
	mount -t ubifs ubi:rootfs_ubifs / -o remount,rw ;
fi;
