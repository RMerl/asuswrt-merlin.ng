#!/bin/sh

if grep -q rootfs_ubifs /proc/mtd ; # grab line with "rootfs_ubifs"
then # UBIFS partition "rootfs_ubifs" was found
	mount -t ubifs -o remount,rw ubi:rootfs_ubifs / ;
fi;
