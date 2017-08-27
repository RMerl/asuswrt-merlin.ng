#!/bin/sh

case "$1" in
	start)
		echo "Mounting filesystems..."
		/bin/mount -a

		# Mount point for determining image version
#		mkdir /mnt/fs_update

		# Mount /dev/pts as devpts
		mkdir /dev/pts
		mount -t devpts devpts /dev/pts

		# Configure hotplug handler
		#echo /rom/etc/hp_wrapper.sh > /proc/sys/kernel/hotplug

		# Initialize and run mdev to crete dynamic device nodes
		echo ">>>>> Starting mdev <<<<<"
		/sbin/mdev -s

		# Create static device nodes
		/rom/etc/make_static_devnodes.sh

		echo ">>>>> Mounting /data partition <<<<<"
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
			else # otherwise mount as JFFS2
				echo ">>>>> Mounting data partition as JFFS2 <<<<<"
				mount -t jffs2 mtd:data /data -rw;
			fi
		else # otherwise if this is not a pureUBI image mount data partition as JFFS2 to allow for backwards compatibility
			echo ">>>>> Mounting data partition as JFFS2 <<<<<"
			mount -t jffs2 mtd:data /data -rw;
		fi

		#echo ">>>>> Mounting misc2 partition as JFFS2 <<<<<"
		#mount -t jffs2 mtd:misc2 /jffs -rw;

		mknod /var/fuse c 10 229
		chmod a+rw /var/fuse
		mkdir -p /var/log /var/run /var/state/dhcp /var/ppp /var/udhcpd /var/zebra /var/siproxd /var/cache /var/tmp /var/samba /var/samba/share /var/samba/homes /var/samba/private /var/samba/locks
		ln -s /var/log/log /dev/log
		cp  /rom/etc/smb.conf /var/samba/ 2>/dev/null
		exit 0
		;;

	stop)
		sync
		echo "Unmounting filesystems..."
		/bin/umount -a
		exit 0
		;;

	*)
		echo "$0: unrecognized option $1"
		;;

esac

