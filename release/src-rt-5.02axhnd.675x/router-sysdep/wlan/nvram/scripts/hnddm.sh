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

DM_UBI_VOLUME=crash_logs

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
	if [ "$?" == "0" ] && [ "$MTD" != "" ]; then
		echo "[$0]: Found $PARTITION partition on MTD device $MTD"
		echo "[$0]: Attaching MTD device $MTD"
		ubiattach -m $MTD || error_exit "$0: ubiattach failed"
		if [ ! -d $MOUNT_DIR ]; then
			echo "[$0]: Creating $MOUNT_DIR"
			mkdir -p $MOUNT_DIR || error_exit "$0: mkdir failed"
		else
			echo "[$0]: $MOUNT_DIR exists"
		fi
		UBI=`grep -l $MTD /sys/class/ubi/*/mtd_num`
		UBI=${UBI/\/mtd_num/}; # remove "/mtd_num" from the path
		UBI=${UBI/\/sys\/class\/ubi\//}; # remove "/sys/class/ubi/" from the path
		echo "[$0]: Mounting DM UBI volume"
		mount -t ubifs $UBI:$DM_UBI_VOLUME $MOUNT_DIR
		
		if [ "$?" != "0" ]; then
			ubidetach -m $MTD
			# Creating UBI volume
			echo "[$0]: Erasing the MTD $MTD partition"
			ubiformat -y /dev/mtd$MTD || error_exit "$0: ubiformat failed"
			ubiattach -m $MTD || error_exit "$0: ubiattach failed"
			DEV=/dev/$UBI # mdev should already populate the new device node for ubi in /dev.
			echo "[$0]: DEV=$DEV UBI=$UBI"
			[ -e $DEV ] || error_exit "$0: device node '$DEV' does not exist."
			echo "[$0]: Creating new UBI volume"
			ubimkvol $DEV --name=$DM_UBI_VOLUME --type=dynamic --maxavsize || error_exit "$0: ubimkvol failed"
			echo "[$0]: Mounting DM UBI volume"
			mount -t ubifs $UBI:$DM_UBI_VOLUME $MOUNT_DIR || error_exit "$0: mount failed"
		fi
		
		return 0
	else
MSG_FATAL_NAND_MISCPARTITION=$(cat <<-END
    ***  Cannot find $PARTITION partition ***.

    Make sure that '$PARTITION' partition size in CFEROM image for debug_monitor
    has been configured.
    If so, then make sure that '$PARTITION' partition size is specifed in CFEROM
    NVRAM on the NANAD flash.
    Use CFERAM 'c' command to specify $PARTITION partition size in CFEROM
    NVRAM on the NANAD flash.
END
)
		echo "[$0]: $MSG_FATAL_NAND_MISCPARTITION"
		exit 1
	fi
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
			mke2fs -t $FSTYPE -F $DEV

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

	# Unmounting and detaching misc1 volume.
	umount $UMOUNT_DIR

	if [ "$FLASHTYPE" == "NAND" ]; then
		get_MTD "$UMOUNT_PART"
		if [ "$?" == "0" ] && [ "$MTD" != "" ]; then
			echo "[$0]: Detaching MTD device $MTD from UBI"
			ubidetach -m $MTD
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

if [ /dev/root -ef /dev/rootfs1 ] || [ /dev/root -ef /dev/rootfs2 ]; then
	FLASHTYPE="eMMC"
	FSTYPE=ext4
elif [ /dev/root -ef /dev/mtdblock0 ]; then 
	FLASHTYPE="NOR"
else
	FLASHTYPE="NAND"
	FSTYPE=ubifs
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