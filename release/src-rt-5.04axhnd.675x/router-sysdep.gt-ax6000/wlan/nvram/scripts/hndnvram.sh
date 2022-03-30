#!/bin/sh

echo "hndnvram.sh: mount /tmp/mnt/defaults"
mkdir /tmp/mnt/defaults
mount -t ubifs ubi:defaults /tmp/mnt/defaults

MFG_NVRAM_PARTITION=misc1
DEFAULTS_MNT_DIR=/mnt/defaults
DEFAULTS_DIR_MFG_NVRAM=$DEFAULTS_MNT_DIR/wl
DIR_MFG_NVRAM=/mnt/nvram
FILE_MFG_NVRAM=nvram.nvm
build_rdk=BUILD_RDKWIFI_HOLDER
original_kernel_nvram_file="/rom/etc/wlan/KERNEL_NVRAM_FILE_NAME"
kernel_nvram_file="/data/.KERNEL_NVRAM_FILE_NAME"
wl_srom_nm=".wlsromcustomerfile.nvm"    #wl calibration file name
SYSTEM_INFO_INDICATOR=$(cat /proc/nvram/wl_nand_manufacturer)
SYSTEM_UBOOT=$(($SYSTEM_INFO_INDICATOR & 8))

# user_nvram_file is required for UNFWLCFG. In BASESHELL build,
# we would store nvram configurations accessed by userspace into the file
user_nvram_file=""

#
# Mounts manufacturing NVRAM UBI FS on dedicated MTD partition.
# ["-r"] option mounts the filesystem read-only.
#
mount_nand_mfg_nvram_fs()
{
    
    
    if MTD=`grep $MFG_NVRAM_PARTITION /proc/mtd`;
    then
	MTD=${MTD/mtd/}; # replace "mtd" with nothing
	MTD=${MTD/:*/}; # replace ":*" (trailing) with nothing
	echo "[$0]: Found $MFG_NVRAM_PARTITION partition on MTD devide $MTD"

	echo "[$0]: Attaching MTD device $MTD"
	ubiattach -m $MTD
	if [ "$?" != "0" ]; then
	    echo "*** ERROR $0: ubiattach failed"  
	    return 1
	fi
	
	# Avoid potential race condition.
	# "ubiattach" command hasn't yet finish to open the volume "ubi1",
	# meanwhile the "mount" command would try to open volume "ubi1_0".
	sleep 1

	echo "[$0]: Mounting nvram UBI volume"
	mkdir $DIR_MFG_NVRAM
	if [ "$?" != "0" ]; then
	    echo "*** ERROR $0: mkdir failed"
	    ubidetach -m $MTD
	    return 1
	fi

	UBI=`grep -l $MTD /sys/class/ubi/*/mtd_num`
	UBI=${UBI/\/mtd_num/}; # remove "/mtd_num" from the path
	UBI=${UBI/\/sys\/class\/ubi\//}; # remove "/sys/class/ubi/" from the path
	mount -t ubifs $UBI:nvram $DIR_MFG_NVRAM $1
	if [ "$?" != "0" ]; then
	    echo "*** ERROR $0: mount as $FS_ACCESS_TYPE failed."
	    ubidetach -m $MTD
	    [ -d $DIR_MFG_NVRAM ] && rm -rf $DIR_MFG_NVRAM
	    return 1
	fi
    else
	echo "*** $0: \"$MFG_NVRAM_PARTITION\" partition is not found."
	return 1
    fi

    echo "[$0]: Done"
    return 0
}

umount_nand_mfg_nvram_fs()
{
    echo "[$0]: Un-Mounting manufacturing default NVRAM fs on MTD partition $MFG_NVRAM_PARTITION..."

    if MTD=`grep $MFG_NVRAM_PARTITION /proc/mtd`;
    then

	MTD=${MTD/mtd/}; # replace "mtd" with nothing
	MTD=${MTD/:*/}; # replace ":*" (trailing) with nothing
	echo "[$0]: $MFG_NVRAM_PARTITION is on MTD devide $MTD"

	echo "[$0]: Un-mounting nvram UBI volume"
	umount $DIR_MFG_NVRAM
	if [ "$?" != "0" ]; then
	    echo "*** ERROR $0: umount failed."
	    return 1
	fi
	
	[ -d $DIR_MFG_NVRAM ] && rm -rf $DIR_MFG_NVRAM
	
	echo "[$0]: Detaching MTD device $MTD from UBI"
	ubidetach -m $MTD
	if [ "$?" != "0" ]; then
	    echo "*** ERROR $0: ubidetach failed."
	    return 1
	fi
    else
	echo  "*** $0: $MFG_NVRAM_PARTITION partition does not exist"
	return 1
    fi

    echo "[$0]: Done"
    return 0
}

mount_emmc_mfg_nvram_fs()
{
	echo "[$0]: Mounting manufacturing default NVRAM ext4 fs on eMMC partition $MFG_NVRAM_PARTITION as $FS_ACCESS_TYPE..."

	# eMMC flash default file system is ext4
	DEV=/dev/$MFG_NVRAM_PARTITION

	# Check if the symbolic link to the emmc misc1 partition exists
	if [ -L $DEV ]; then
	    # Mounting misc partition
	    echo "[$0]: Mounting eMMC $MFG_NVRAM_PARTITION Partition ..." 
	    mkdir -p $DIR_MFG_NVRAM
	    mount -t $FSTYPE $DEV $DIR_MFG_NVRAM $1

	    if [ "$?" != "0" ]; then
	        echo "*** ERROR $0: mount failed."
	        return 1
	    fi
	else
	    echo "*** $0: \"$MFG_NVRAM_PARTITION\" partition is not found."
	    return 1
	fi

	echo "[$0]: Done"
	return 0
}

umount_emmc_mfg_nvram_fs()
{
    echo "[$0]: Un-Mounting manufacturing default NVRAM fs on MMC partition $MFG_NVRAM_PARTITION..."

    DEV=/dev/$MFG_NVRAM_PARTITION
    FSTYPE=ext4

    # Check if the symbolic link to the emmc misc1 partition exists
    if [ -L $DEV ]; then
	    echo "[$0]: Un-mounting nvram eMMC volume"
	    umount $DIR_MFG_NVRAM
	    if [ "$?" != "0" ]; then
	        echo "*** ERROR $0: umount failed."
	        return 1
	    fi
	
	    [ -d $DIR_MFG_NVRAM ] && rm -rf $DIR_MFG_NVRAM
    else
	    echo  "*** $0: $MFG_NVRAM_PARTITION partition does not exist"
	    return 1
    fi

    echo "[$0]: Done"
    return 0
}

mount_nor_mfg_nvram_fs()
{
    if MTD=`grep $MFG_NVRAM_PARTITION /proc/mtd`;
    then
        MTD=${MTD/mtd/}; # replace "mtd" with nothing
        MTD=${MTD/:*/}; # replace ":*" (trailing) with nothing
        echo "[$0]: Found $MFG_NVRAM_PARTITION partition on MTD device $MTD"

        mkdir $DIR_MFG_NVRAM
        mount -t jffs2 mtd:$MFG_NVRAM_PARTITION $DIR_MFG_NVRAM $1
        if [ "$?" != "0" ]; then
            echo "*** ERROR $0: mount as $FS_ACCESS_TYPE failed."
            [ -d $DIR_MFG_NVRAM ] && rm -rf $DIR_MFG_NVRAM
            return 1
        fi
    else
        echo "*** $0: \"$MFG_NVRAM_PARTITION\" partition is not found."
        return 1
    fi

    echo "[$0]: Done"
    return 0
}

umount_nor_mfg_nvram_fs()
{
    echo "[$0]: Un-Mounting manufacturing default NVRAM fs on MTD partition $MFG_NVRAM_PARTITION..."

    if MTD=`grep $MFG_NVRAM_PARTITION /proc/mtd`;
    then

        MTD=${MTD/mtd/}; # replace "mtd" with nothing
        MTD=${MTD/:*/}; # replace ":*" (trailing) with nothing
        echo "[$0]: $MFG_NVRAM_PARTITION is on MTD devide $MTD"

        echo "[$0]: Un-mounting nvram"
        umount $DIR_MFG_NVRAM
        if [ "$?" != "0" ]; then
            echo "*** ERROR $0: umount failed."
            return 1
        fi
	
        [ -d $DIR_MFG_NVRAM ] && rm -rf $DIR_MFG_NVRAM
    else
        echo  "*** $0: $MFG_NVRAM_PARTITION partition does not exist"
        return 1
    fi

    echo "[$0]: Done"
    return 0
}
nvram_mfg_restore_default()
{
    echo "[$0]: Restore NVRAM from default mfg NVRAM file"
    nvram restore_mfg $kernel_nvram_file
    sync
    return 0
}

populate_nvram()
{
    nvram kernelset $kernel_nvram_file
    sync
    return 0
}

check_mfg_nvram_partition()
{
	if [ $FLASHTYPE == "NAND" ]; then
	    # NAND flash
	    MISC=`grep -l  "$MFG_NVRAM_PARTITION" /sys/class/mtd/mtd*/name`
	    MISC=${MISC/*mtd/};     # replace "*mtd" with nothing
	    MISC=${MISC/\/name/};   # replace /name with nothing
	    if grep "^$MISC$" /sys/class/ubi/ubi*/mtd_num
	    then
	        return 1
	    fi
	    MISC=`grep "^$MISC$" /sys/class/ubi/ubi*/mtd_num`
	elif [ $FLASHTYPE == "NOR" ]; then
	    if grep $MFG_NVRAM_PARTITION /proc/mtd
	    then
	        return 0
	    else
	        return 1
	    fi
	else
	    # eMMC flash
	    MISC=/dev/$MFG_NVRAM_PARTITION
	    if [ L $MISC ]; then
	        return 1
	    fi
	fi

	return 0
}

#$1 remout options rw/ro
mfg_remount_defaults()
{
	echo "MOUNT defaults for " $1
	if [ "$FLASHTYPE" == "eMMC" ]; then
		mount -t $FSTYPE -o remount,$1 /dev/defaults $DEFAULTS_MNT_DIR
	elif [ $FLASHTYPE == "NAND" ]; then
		mount -t ubifs ubi:defaults $DEFAULTS_MNT_DIR -oremount,$1
	fi
}

mount_mfg_nvram_fs()
{
	if [ "$NEEDMOUNT" == "1" ]
	then

	    [ $# -ne 0 ] && [ "$1" != "-r" ] && echo "[$0]: *** invalid argument $1" && return 1
	    FS_ACCESS_TYPE="read/write"
	    [ "$1" == "-r" ] && FS_ACCESS_TYPE="read-only"
	    echo "[$0]: Mounting manufacturing default NVRAM UBI fs on MTD partition $MFG_NVRAM_PARTITION as $FS_ACCESS_TYPE..."

	    check_mfg_nvram_partition

	    if [ "$?" != "1" ]; then
	        # mounting manufacturing NVRAM UBI file system as read/write.
	        if [ $FLASHTYPE == "NAND" ]; then
	            mount_nand_mfg_nvram_fs
	        elif [ $FLASHTYPE == "eMMC" ]; then
	            mount_emmc_mfg_nvram_fs
	        else
	            mount_nor_mfg_nvram_fs
	        fi
	    fi
	else
	    # uBoot, /mnt/nvram -> /mnt/defaults has been already present
	    # make /mnt/defaults writable
	    if [ -d $DEFAULTS_MNT_DIR ]; then
		mfg_remount_defaults "rw"
	    fi
	fi

	return $?
}

umount_mfg_nvram_fs()
{
	if [ "$NEEDUMOUNT" != "0" ]; then
	    # un-mounting manufacturing NVRAM file system.
	    echo "[$0]: Un-mounting manufacturing default NVRAM fs on $MFG_NVRAM_PARTITION partition"

	    if [ $FLASHTYPE == "NAND" ]; then
	        umount_nand_mfg_nvram_fs
	    elif [ $FLASHTYPE == "eMMC" ]; then
	        umount_emmc_mfg_nvram_fs
	    else
	        umount_nor_mfg_nvram_fs
	    fi
	else
	    rm $DIR_MFG_NVRAM
	    # make /mnt/defaults read-only if write_defaults is not set
	    if [ -d $DEFAULTS_MNT_DIR ] && [ ! -r /proc/environment/write_defaults ]; then
		mfg_remount_defaults "ro"
	    fi
	fi
	echo "umount_mfg_nvram_fs"
	sync
	return 0
}


# Check for defaults
if [ -d $DEFAULTS_MNT_DIR ]
then
    echo "$DEFAULTS_MNT_DIR present"
    mkdir -p $DEFAULTS_DIR_MFG_NVRAM
    [ ! -L $DIR_MFG_NVRAM ] && ln -s $DEFAULTS_DIR_MFG_NVRAM $DIR_MFG_NVRAM
    MNTNOK=0
    NEEDMOUNT=0
    NEEDUMOUNT=0
else
    NEEDMOUNT=1
fi

# Get flash type
if  [[ $SYSTEM_UBOOT -gt 0 ]]; then 
    _root_fs_dev=`/rom/etc/get_rootfs_dev.sh`
    if [[ ! -z $(echo $_root_fs_dev|grep mmcblk) ]]; then
        FLASHTYPE="eMMC"
        FSTYPE=ext4
    elif [[ ! -z $(echo $_root_fs_dev|grep ubi)  ]]; then 
        FLASHTYPE="NAND"
        FSTYPE=ubifs
    else
        FLASHTYPE="NOR"
        FSTYPE=jffs2
        MFG_NVRAM_PARTITION=data
    fi
else
	if [ /dev/root -ef /dev/rootfs1 ] || [ /dev/root -ef /dev/rootfs2 ]; then
		FLASHTYPE="eMMC"
		FSTYPE=ext4
	elif [ /dev/root -ef /dev/mtdblock0 ]; then
		FLASHTYPE="NOR"
		FSTYPE=jffs2
		MFG_NVRAM_PARTITION=data
	else
		FLASHTYPE="NAND"
		FSTYPE=ubifs
	fi
fi


if [ "$FLASHTYPE" == "" ]; then
	echo "[$0]: Un supported flash type, exiting"
	exit 1
fi

case "$1" in
    start)

	insmod /lib/modules/*/extra/wlcsm.ko
	
	if  [ ! -e $kernel_nvram_file ]; then
	    # If kernel nvram does not exist, trying to restore
	    # it from default manufacturing NVRAM file.
		cp $original_kernel_nvram_file $kernel_nvram_file
		sync

		if [ "$NEEDMOUNT" == "1" ]
		then
	    		# defaults does not exist, need to mount
	    		mount_mfg_nvram_fs
	    		MNTNOK=$? 
	    		NEEDUMOUNT=1
		fi
		if [ "$MNTNOK" != "0" ]; then
	    		# mount fails.
	    		echo "*** Could not mount mfg partition"
            		STILLNOK=1
		else
	    
	    		if  [ -f $DIR_MFG_NVRAM/$FILE_MFG_NVRAM ]; then
	        		# restore from default manufacturing NVRAM file.
	        		nvram_mfg_restore_default
	    		else
	        		echo  "*** Manufacturing NVRAM file doen not exist. ***"
                		STILLNOK=1
	    		fi

	    		umount_mfg_nvram_fs
		fi
	fi

	if  [ ! -f /data/$wl_srom_nm ]; then
	    # If custom calibrated nvram does not exist, trying to restore
	    # it from default location.
	    if [ -f $DEFAULTS_DIR_MFG_NVRAM/$wl_srom_nm ]; then
		echo "/data/$wl_srom_nm does not exist, restore from $DEFAULTS_DIR_MFG_NVRAM/$wl_srom_nm"
		cp -f $DEFAULTS_DIR_MFG_NVRAM/$wl_srom_nm /data/$wl_srom_nm
		sync
	    fi
	fi
	if  [[ $SYSTEM_UBOOT -gt 0 ]]; then 
	   image_default_nvram_file="/rom/etc/wlan/nvram/"`cat /proc/environment/boardid`".nvm"
	   if [ -f /proc/environment/wlnvram ]; then
		image_default_nvram_file="/rom/etc/wlan/nvram/"`cat /proc/environment/wlnvram`
	   fi
	else
	   image_default_nvram_file="/rom/etc/wlan/nvram/"`cat /proc/nvram/boardid`".nvm"
	fi
	if  [ -n "$STILLNOK" ] && [ -f $image_default_nvram_file ]; then
	   echo  "*** Using $image_default_nvram_file NVRAM file from image ***"
	   mkdir -p $DIR_MFG_NVRAM
		cp $image_default_nvram_file $DEFAULTS_DIR_MFG_NVRAM/$FILE_MFG_NVRAM
	   ln -sf $image_default_nvram_file $DIR_MFG_NVRAM/$FILE_MFG_NVRAM
	   nvram_mfg_restore_default
	fi

	if  [ -f $kernel_nvram_file ]; then
	    populate_nvram
	     echo  "*** populated ***"
	fi

	if  [ ! -z $user_nvram_file ] && [ ! -f $user_nvram_file ]; then
	    # If user nvram file does not exist, jsut touch it
		echo -n "" > $user_nvram_file
		sync
	fi
	
	if [ $build_rdk -eq 1 ]; then
		mkdir -p /data/rdklogs/logs
	fi

	exit 0
	;;
    stop)
	rmmod /lib/modules/*/extra/wlcsm.ko
	exit 0
	;;

    mount_ubi)
	    mount_mfg_nvram_fs $2
	exit $?
	;;

    umount_ubi)
	    umount_mfg_nvram_fs
	exit $?
	;;

    *)
	echo "$0: unrecognized option $1"
	;;

esac
