#!/bin/sh

#function is_file_to_delete() {
##############################################################
# Append this list with all non removable files in /data 
#    local files_to_skip="reg_id pmd_calibration.json pmd_calibration pmd_temp2apd"
##############################################################

#    local file=$1
#    for elem in $files_to_skip; do
#        if [ $file == $elem ] ; then 
#            return 1  # 1 - false 
#        fi
#    done
#    return 0 # 0 - true
#}

files_to_skip="reg_id pmd_calibration.json pmd_calibration pmd_temp2apd"

case "$1" in
	start)
		echo "Mounting filesystems..."
		/bin/mount -a

		if grep -q '^bootfs$' /sys/class/mtd/mtd*/name
		then
		    mount -t jffs2 mtd:bootfs /bootfs -oro
		fi

		# Mount point for determining image version
		# mkdir /mnt/fs_update

		# Mount /dev/pts as devpts
		mkdir /dev/pts
		mount -t devpts devpts /dev/pts

		# Configure hotplug handler
		# echo /rom/etc/hp_wrapper.sh > /proc/sys/kernel/hotplug

		# Initialize and run mdev to crete dynamic device nodes
		echo ">>>>> Starting mdev <<<<<"
		/sbin/mdev -s

		# Create static device nodes
		/rom/etc/make_static_devnodes.sh

		echo ">>>>> Mounting /data partition <<<<<"
		# Check if our rootfs is pointing to any of the emmc rootfs partitions
		if [ /dev/root -ef /dev/rootfs1 ] || [ /dev/root -ef /dev/rootfs2 ]; then
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
		else
			# if mounting data partition as UBIFS fails then mount as JFFS2
			if TEMP=`grep METADATA /proc/mtd` && MTD=`grep data /proc/mtd`; # if pure UBI image grab line with "data"
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
				#else # otherwise mount as JFFS2
					#echo ">>>>> Mounting data partition as JFFS2 <<<<<"
					#mount -t jffs2 mtd:data /data -rw;
				fi
			else # otherwise if this is not a pureUBI image mount data partition as JFFS2 to allow for backwards compatibility
				echo ">>>>> Mounting data partition as JFFS2 <<<<<"
				mount -t jffs2 mtd:data /data -rw;
			fi
		fi

		mknod /var/fuse c 10 229
		chmod a+rw /var/fuse
		mkdir -p /var/log /var/run /var/state/dhcp /var/ppp /var/udhcpd /var/zebra /var/siproxd /var/cache /var/tmp /var/samba /var/samba/share /var/samba/homes /var/samba/private /var/samba/locks
		ln -s /var/log/log /dev/log
		cp  /rom/etc/smb.conf /var/samba/ 2>/dev/null
        for i in $(cat /proc/cmdline) ; do
           case $i in 
              erase_psi)
              echo ">>>> Cleanup PSI was requested <<<<"
              for t in $(ls -A /data) ; do
		 for elem in $files_to_skip; do
			if [ $t == $elem ] ; then 
				echo "Removing /data/$t"
				rm -rf /data/$t
			fi
		done
              done
              ;;
           esac
        done

		exit 0
		;;

	stop)
		sync
		echo "Unmounting filesystems..."
		/bin/umount -a -l
		exit 0
		;;

	*)
		echo "$0: unrecognized option $1"
		;;

esac

