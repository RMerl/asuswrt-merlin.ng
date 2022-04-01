#!/bin/sh


## create ubi volumn by input vol name, size, ubinum 
## if vol does not exists, it will try to create if desired vol size is positive.
## if vol exists already and its size is not matching to the uboot environment size,
## it will recreae it if env vol_resize_enable exists. Otherwize, it will do nothing 
##  but using current vol. 
mk_ubi_vol() {
	local vol_name=$1
	local default_vol_size=$2
	local vol_desired_num=$3
	local vol=`grep '^'$vol_name'$' /sys/class/ubi/ubi0_*/name`
	local vol_current_size=0
	if [ ! -z $vol ]; then 
		vol=${vol%$vol_name}
		vol=${vol//ubi0_/}
		vol_desired_num=${vol//[!0-9]/}
		vol_current_size=`cat /sys/class/ubi/ubi0_$vol_desired_num/data_bytes`
		vol_current_size=$(($vol_current_size/1024/1024))
		echo "vol:$vol; vol_desired_num:$vol_desired_num; vol_current_size:$vol_current_size;"
	else
		#make sure the new volum number is availabel 
		while [ -r /sys/class/ubi/ubi0_$vol_desired_num ];
		do
			vol_desired_num=$(($vol_desired_num+1))
		done
		echo "New vol num: vol_desired_num:$vol_desired_num;"
	fi
	local uboot_vol_size='/proc/environment/'$vol_name'_vol_size'
	echo "vol_name:$1; default_vol_size:$2; vol_desired_num:$3; vol:$vol;"
	echo "desired uboot_vol_size of ($vol_name):$uboot_vol_size"
	if [ -r $uboot_vol_size ]; then
		vol_desired_size=`cat $uboot_vol_size`
		echo "desire as uboot env"
	else
		vol_desired_size=$default_vol_size
		echo "desire as def"
	fi
	local vol_resize_enable=0
	if [ -r /proc/environment/vol_resize_enable ];then
		vol_resize_enable=1
	fi
	echo "vol_resize_enable:$vol_resize_enable"
	if [ $vol_desired_size -gt 0 ]; then
		if [ $vol_desired_size -eq $vol_current_size ];then
			echo "VOL:$vol_name already exists with ${vol_desired_size}M."
		else
			echo " VOL:$vol_name size: ${vol_current_size}Mb  and desired: ${vol_desired_size}Mb."
			if [ $vol_current_size -gt 0 ] && [ $vol_resize_enable -eq 1 ]; then
				echo "remove $vol_name and will recreate"
				ubirmvol /dev/ubi0 -n $vol_desired_num 2>/dev/null
			fi
			if [ $vol_current_size -eq 0 ] || [ $vol_resize_enable -eq 1 ]; then
				ubimkvol /dev/ubi0 -s ${vol_desired_size}MiB -n $vol_desired_num -N $vol_name 2>/dev/null
				if [ $? -ne 0 ];then
					echo "create VOL:$vol_name failure!!!!"
					return 1; # -false
				fi
			fi
		fi
	else
		if [ $vol_current_size -eq 0 ]; then
			#not needed to do anything for this volume as it request ZERO size and real size ZERO
			echo "Nothing to be done"
		elif  [ $vol_resize_enable -eq 1 ]; then
			echo "remove $vol_name as desired size is 0"
			ubirmvol /dev/ubi0 -n $vol_desired_num 2>/dev/null
		else
			echo "desired size is 0 and rel size is not, but NOT resize"
		fi
		return 1; # -false
	fi
	return 0;
}

mount_data() {
		echo ">>>>> Mounting /data partition start <<<<<"
		# Check if our rootfs is pointing to any of the emmc rootfs partitions
		if TEMP=`grep mmcblk /proc/cmdline`; then
			# For new flash layout mount mandatory defaults partition
			if [ ! -L /dev/nvram ]; then
				echo ">>>>> For new flash layout mount mandatory defaults partition <<<<<"
				if [ ! -r /proc/environment/write_defaults ] && [ ! -r /proc/environment/mfg_nvram_mode ]
				then
					dfltopt="-oro"
				fi
				mkdir -p /mnt/defaults
				if [ -L /dev/defaults ]; then
					echo ">>>>> Mounting eMMC defaults Partition <<<<<"
					mount -t ext4 /dev/defaults /mnt/defaults -rw;
					if [ ! $? -eq 0 ]; then
						echo ">>>>> Formatting eMMC defaults Partition <<<<<"
						mke2fs -t ext4 -F /dev/defaults;
						mount -t ext4 /dev/defaults /mnt/defaults $dfltopt;
					fi
				else
					echo ">>>>> ERROR: Failed to mount eMMC defaults partition <<<<<"

				fi
			fi

			echo ">>>>> eMMC rootfs <<<<<"
			# Check if the symbolic link to the emmc data partition exists
			if [ -L /dev/data ]; then
				echo ">>>>> Mounting eMMC Data Partition <<<<<"
				mount -t ext4 /dev/data /data -rw;
				if [ ! $? -eq 0 ]; then
					echo ">>>>> Formatting eMMC Data Partition <<<<<"
					mke2fs -t ext4 -F /dev/data;
					mount -t ext4 /dev/data /data -rw;
				fi
			else
				echo ">>>>> ERROR: Failed to mount eMMC Data partition <<<<<"

			fi

			# Check if the symbolic link to the emmc misc partition exists
			if [ -L /dev/misc ]; then
				echo ">>>>> Mounting eMMC misc Partition <<<<<"
				mkdir -p /mnt/misc
				mount -t ext4 /dev/misc /mnt/misc -rw;
				if [ ! $? -eq 0 ]; then
					echo ">>>>> Formatting eMMC misc Partition <<<<<"
					mke2fs -t ext4 -F /dev/misc;
					mount -t ext4 /dev/misc /mnt/misc -rw;
				fi
				mkdir -p /mnt/misc/crash_logs
				ln -s /mnt/misc/crash_logs /mnt/crash_logs
			fi
		else
			if TEMP=`grep spi-nor.0 /proc/cmdline`
			then
				echo ">>>>> Mounting data partition as JFFS2 <<<<<"
				mount -t jffs2 mtd:data /data -rw;
			elif TEMP=`grep "\bloader" /proc/mtd` && MTD=`grep image /proc/mtd`;
			then
				# /data volume is the most important volume, create and mount first
				if mk_ubi_vol data __BRCM_DATA_VOLUME_SIZE__ 10; then
					mount -t ubifs ubi:data /data
				fi
				if mk_ubi_vol defaults __BRCM_DEFAULTS_VOLUME_SIZE__  11; then
					mkdir -p /mnt/defaults
					mount -t ubifs ubi:defaults /mnt/defaults
					if [ ! -r /proc/environment/write_defaults ] && [ ! -r /proc/environment/mfg_nvram_mode ]; then
						mount -t ubifs ubi:defaults /mnt/defaults -oremount,ro
					fi
				fi
				if mk_ubi_vol misc 0 12; then 
					mkdir -p /mnt/misc
					mount -t ubifs ubi:misc /mnt/misc
					mkdir -p /mnt/misc/crash_logs
					ln -s /mnt/misc/crash_logs /mnt/crash_logs
				fi
				echo ">>>>> add jffs2 UBI Volume for /jffs partition <<<<<"
				if mk_ubi_vol jffs2 __ASUS_JFFS2_VOLUME_SIZE__ 13; then
					mount -t ubifs ubi:jffs2 /jffs
				fi
			elif TEMP=`grep METADATA /proc/mtd` && MTD=`grep data /proc/mtd`; # if pure UBI image grab line with "data"
			# if mounting data partition as UBIFS fails then mount as JFFS2
			then # if this is a pureUBI image attempt to create/mount data partition as UBI if none exists or already UBI
				MTD=${MTD/mtd/}; # replace "mtd" with nothing
				MTD=${MTD/:*/}; # replace ":*" (trailing) with nothing
				if UBI=`ubiattach -m "$MTD"`; 	# try to attach data partition mtd to ubi, will format automatically if empty, 
								# will attach if UBI, will fail if not empty with no UBI 
								# (i.e. JFFS2 has previously mounted this partition and written something)
				then # ubiattach was successful, mount UBI
					echo ">>>>> Mounting data partition as UBIFS <<<<<"
					UBI=${UBI##*"number "}; # cut all before "number ", still need to get rid of leading space
					UBI=${UBI%%,*}; # cut all after ","
					DATA_PNAME=`ubinfo /dev/ubi"$UBI" -a | grep -o data`;
					# if data partition already exists, do not invoke ubimkvol
					if [ "$DATA_PNAME" != "data" ]; then
						echo ">>>>> Creating ubi volume ubi$UBI:data <<<<<"
						ubimkvol /dev/ubi"$UBI" -m -N data; 
					fi
					mount -t ubifs ubi"$UBI":data /data;
				else # otherwise mount as JFFS2
					echo ">>>>> Mounting data partition as JFFS2 <<<<<"
					mount -t jffs2 mtd:data /data -rw;
				fi
			else # otherwise if this is not a pureUBI image mount data partition as JFFS2 to allow for backwards compatibility
				echo ">>>>> Mounting data partition as JFFS2 <<<<<"
				mount -t jffs2 mtd:data /data -rw;
			fi
		fi
		return 0;
}

