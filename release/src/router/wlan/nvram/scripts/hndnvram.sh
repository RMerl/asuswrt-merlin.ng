#!/bin/sh

MTD_NVRAM_PARTITION=misc1
DIR_MFG_NVRAM=/mnt/nvram
FILE_MFG_NVRAM=nvram.nvm
kernel_nvram_file="/data/.KERNEL_NVRAM_FILE_NAME"

#
# Mounts manufacturing NVRAM UBI FS on dedicated MTD partition.
# ["-r"] option mounts the filesystem read-only.
#
mount_mfg_nvram_ubi_fs()
{
    [ $# -ne 0 ] && [ "$1" != "-r" ] && echo "[$0]: *** invalid argument $1" && return 1

    FS_ACCESS_TYPE="read/write"
    [ "$1" == "-r" ] && FS_ACCESS_TYPE="read-only"
    echo "[$0]: Mounting manufacturing default NVRAM UBI fs on MTD partition $MTD_NVRAM_PARTITION as $FS_ACCESS_TYPE..."
    
    
    if MTD=`grep $MTD_NVRAM_PARTITION /proc/mtd`;
    then
	MTD=${MTD/mtd/}; # replace "mtd" with nothing
	MTD=${MTD/:*/}; # replace ":*" (trailing) with nothing
	echo "[$0]: Found $MTD_NVRAM_PARTITION partition on MTD devide $MTD"

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
	echo "*** ERROR $0: \"$MTD_NVRAM_PARTITION\" partition is not found."
	return 1
    fi

    echo "[$0]: Done"
    return 0
}

umount_mfg_nvram_ubi_fs()
{
    echo "[$0]: Un-Mounting manufacturing default NVRAM fs on MTD partition $MTD_NVRAM_PARTITION..."

    if MTD=`grep $MTD_NVRAM_PARTITION /proc/mtd`;
    then

	MTD=${MTD/mtd/}; # replace "mtd" with nothing
	MTD=${MTD/:*/}; # replace ":*" (trailing) with nothing
	echo "[$0]: $MTD_NVRAM_PARTITION is on MTD devide $MTD"

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
	echo  "*** ERROR $0: $MTD_NVRAM_PARTITION partition does not exist"
	return 1
    fi

    echo "[$0]: Done"
    return 0
}

nvram_mfg_restore_default()
{
    echo "[$0]: Restore NVRAM from default mfg NVRAM file"
    nvram restore_mfg $kernel_nvram_file
    return 0
}

populate_nvram()
{
    nvram kernelset $kernel_nvram_file
    return 0
}


case "$1" in
    start)

	insmod /lib/modules/*/extra/wlcsm.ko
	
	kernel_nvram_file_size=`ls -s $kernel_nvram_file`
	kernel_nvram_file_size=${kernel_nvram_file_size/\/*/}; # replace "/*" (trailing) with nothing
	kernel_nvram_file_size=${kernel_nvram_file_size// /}; # replace space with nothing
	echo "kernel_nvram size is ($kernel_nvram_file_size) blocks"
	if  [ ! -e $kernel_nvram_file ] || [ $kernel_nvram_file_size == "0" ]; then
	    # If kernel nvram does not exist, trying to restore
	    # it from default manufacturing NVRAM file.

	    # mounting manufacturing NVRAM UBI file system as read/write.
	    mount_mfg_nvram_ubi_fs
	    if [ "$?" != "0" ]; then
		# mount fails.
		exit 0
	    fi
	    
	    if  [ -f $DIR_MFG_NVRAM/$FILE_MFG_NVRAM ]; then
		# restore from default manufacturing NVRAM file.
		nvram_mfg_restore_default
	    else
		echo  "*** ERROR: Manufacturing NVRAM file doen not exist. ***"
	    fi

	    # un-mounting manufacturing NVRAM UBI file system.
	    umount_mfg_nvram_ubi_fs
	    if [ "$?" != "0" ]; then
		# umount fails.
		exit 0
	    fi
	fi

	if  [ -f $kernel_nvram_file ]; then
	    populate_nvram
	fi

	exit 0
	;;
    stop)
	rmmod /lib/modules/*/extra/wlcsm.ko
	exit 0
	;;

    mount_ubi)
	mount_mfg_nvram_ubi_fs $2
	exit $?
	;;

    umount_ubi)
	umount_mfg_nvram_ubi_fs
	exit $?
	;;

    *)
	echo "$0: unrecognized option $1"
	;;

esac
