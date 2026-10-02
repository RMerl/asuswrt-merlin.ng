#!/bin/sh

#
# USAGE:
#	Mount only and without un-mount
#		- hnddm.sh "partiton" "dir"
#	Re-mount
#		- hnddm.sh "partiiton" "dir" "un-mount partition" "un-mount dir"
#	Un-mount only
#		- hnddm.sh "" "" "un-mount partition" "un-mount dir"
#

SYSTEM_INFO_INDICATOR=$(cat /proc/nvram/wl_nand_manufacturer)
SYSTEM_UBOOT=$(($SYSTEM_INFO_INDICATOR & 8))
DM_UBI_VOL_SIZE=100
DM_UBI_VOL_NAME=misc
DM_UBI_VOL_NUM=12

error_exit()
{
	if [ "$?" != "0" ]; then
		echo "*** ERROR $1"
		exit 1
	fi
}

get_MTD()
{
	local part=$1
	if MTD=`grep $part /proc/mtd`; then
		MTD=${MTD/mtd/}; # replace "mtd" with nothing
		MTD=${MTD/:*/}; # replace ":*" (trailing) with nothing
		return 0
	fi
	return 1
}

mount_nand_dm_fs()
{
	get_MTD "$PARTITION"

	# if misc_vol_size has a value greater than 0, misc partition is created and mounted to /mnt/crash_logs during boot.
	# the base mount point of the misc partition is /mnt/misc.
	# it is referred to mount-data.inc in userspace/private/apps/scripts/std

	if [ "$?" == "0" ] && [ "$MTD" != "" ]; then
	# from /proc/mtd, we can check if the misc partition is already created or mounted.
		echo "[$0]: Found $PARTITION partition on MTD device $MTD"
		echo "[$0]: Already mounted!!"
	else
		if [ "$PARTITION" == "misc" ]; then
			echo "[$0]: Creating DM UBI volume - $DM_UBI_VOL_NAME"

			# ubimkvol will make the volume up to the maximum capacity the volume is supporting
			ubimkvol /dev/ubi0 -s ${DM_UBI_VOL_SIZE}MiB -n $DM_UBI_VOL_NUM -N $DM_UBI_VOL_NAME

			# if it is set, the volume is created and mounted at /mnt/misc during boot.
			# then /mnt/misc/crash_logs is created on /mnt/misc
			echo misc_vol_size=${DM_UBI_VOL_SIZE} > /proc/nvram/set

			echo "[$0]: Reboot is required to ensure proper work"
			echo "[$0]: Creating mount directory $MTD!"

			# create base location
			mkdir -p /mnt/misc
			# mount
			mount -t ubifs ubi:misc /mnt/misc
		fi

		if [ "$PARTITION" == "ram" ]; then
			echo "[$0]: Creating DM RAM DISK"
			mkdir -p /mnt/misc
			mount -t ramfs ramfs /mnt/misc
		fi

	fi

	# $MOUNT_DIR is specified
	# but we only mount /mnt/misc and only use /mnt/misc/crash_logs
	mkdir -p /mnt/misc/crash_logs

	return 0
}

mount_emmc_dm_fs()
{
	DEV=/dev/$PARTITION

	# Check if the symbolic link to the emmc misc partition exists
	if [ -L $DEV ]; then
		if [ ! -d $MOUNT_DIR ]; then
			echo "[$0]: Creating $MOUNT_DIR"
			mkdir -p $MOUNT_DIR || error_exit "$0: mkdir failed"
		fi

		# Mounting misc partition
		echo "[$0]: Mounting eMMC $PARTITION Partition ..." 
		mount -t $FSTYPE $DEV $MOUNT_DIR -rw

		if [ ! $? -eq 0 ]; then
			echo "[$0] Formatting eMMC $PARTITION Partition ..." 
			mke2fs -I 256 -t $FSTYPE -F $DEV

			echo "[$0]: Mounting eMMC $PARTITION Partition ..." 
			mount -t $FSTYPE $DEV $MOUNT_DIR -rw || error_exit "$0: mount failed"
		fi

		return 0
	else
MSG_FATAL_EMMC_MISCPARTITION=$(cat <<-END
    ***  Cannot find $PARTITION partition ***.

    Make sure that '$PARTITION' partition is created from CFE using emmcfmtgpt
    or configured in the image itself.
    Use CFERAM 'showdevs' command to check $PARTITION partition size on the
    eMMC flash.
END
)
		echo "[$0]: $MSG_FATAL_EMMC_MISCPARTITION"
		exit 1
	fi
}

mount_dm_fs()
{
	if [ "$FLASHTYPE" == "NAND" ]; then
		mount_nand_dm_fs
	else
		mount_emmc_dm_fs
	fi
}

umount_dm_fs()
{
	echo "[$0]: Un-mounting DM volume"

	if [ "$UMOUNT_PART" == "ram" ]; then
		umount -t ubifs /mnt/misc
		rm -rf /mnt/misc
		return 0
	fi

	# Unmounting misc volume.
	umount -t ubifs /mnt/misc
	rm -rf /mnt/misc

	if [ "$FLASHTYPE" == "NAND" ]; then
		get_MTD "$UMOUNT_PART"
		if [ "$?" == "0" ] && [ "$MTD" != "" ]; then
			echo "[$0]: Remove MTD device $MTD from UBI"
			#if misc_vol_size is set to 0, the volume won't be mounted
			echo misc_vol_size=0 > /proc/nvram/set
			#explicitly remove the volume
			ubirmvol /dev/ubi0 -n $DM_UBI_VOL_NUM
		fi
	fi

	return 0
}

if [ $# -lt 2 ]; then
	echo "Usage: $0 <partition> <mount dir> [un-mount partition] [un-mount dir]"
	exit 1
fi

PARTITION=$1
MOUNT_DIR=$2
if [ $# -eq 4 ]; then
	UMOUNT_PART=$3
	UMOUNT_DIR=$4
fi

if [[ $SYSTEM_UBOOT -gt 0 ]]; then
    _root_fs_dev=`/rom/etc/get_rootfs_dev.sh`
    if [[ ! -z $(echo $_root_fs_dev|grep mmcblk) ]]; then
        FLASHTYPE="eMMC"
        FSTYPE=ext4
    elif [[ ! -z $(echo $_root_fs_dev|grep ubi)  ]]; then
        FLASHTYPE="NAND"
        FSTYPE=ubifs
    else
        FLASHTYPE="NOR"
    fi
else

	if [ /dev/root -ef /dev/rootfs1 ] || [ /dev/root -ef /dev/rootfs2 ]; then
		FLASHTYPE="eMMC"
		FSTYPE=ext4
	elif [ /dev/root -ef /dev/mtdblock0 ]; then
		FLASHTYPE="NOR"
	else
		FLASHTYPE="NAND"
		FSTYPE=ubifs
	fi
fi

if [ "$FLASHTYPE" == "" ] || [ "$FLASHTYPE" == "NOR" ]; then
	echo "$0: Un supported flash type[$FLASHTYPE], exiting"
	exit 1
fi

if [ ! -z "$UMOUNT_PART" ]; then
	umount_dm_fs
fi

if [ ! -z "$PARTITION" ] && [ "$PARTITION" != "" ]; then
	if [ ! -z "$MOUNT_DIR" ] && [ "$MOUNT_DIR" != "" ]; then
		mount_dm_fs
	fi
fi
exit 0
