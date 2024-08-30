#!/bin/sh

source __DATA_MOUNT_INSTALL_PATH__/mount-data.inc
is_file_to_delete() {
##############################################################
# Append this list with all non removable files in /data 
    local files_to_skip="reg_id pmd_calibration.json pmd_calibration pmd_temp2apd"
##############################################################

    local file=$1
    for elem in $files_to_skip; do
        if [ $file == $elem ] ; then 
            return 1  # 1 - false 
        fi
    done
    return 0 # 0 - true
}

case "$1" in
	start)
		echo "Mounting filesystems..."
		/bin/mount -a

		echo "steadystate" > /proc/bootstate/reset_reason #BCM_BOOT_REASON_WATCHDOG|BCM_BOOT_PHASE_LINUX_RUN
		echo 0 		   > /proc/bootstate/boot_failed_count #reset boot failed out to 0

		if grep -vq spi-nor.0 /proc/cmdline
		then		
		    if grep -q '^bootfs$' /sys/class/mtd/mtd*/name
		    then
		        mount -t jffs2 -oro mtd:bootfs /bootfs
		    fi
		fi    

		# Mount point for determining image version
		mkdir /mnt/fs_update

		# Mount /dev/pts as devpts
		mkdir /dev/pts
		mount -t devpts devpts /dev/pts

		# Configure hotplug handler
		echo > /dev/mdev.seq
		#echo > /dev/mdev.log
		echo /sbin/mdev >/proc/sys/kernel/hotplug

		# Initialize and run mdev to crete dynamic device nodes
		echo ">>>>> Starting mdev <<<<<"
		/sbin/mdev -s

		# Create static device nodes
		/rom/etc/make_static_devnodes.sh

		# add device key to keyring if available
		if [ -x /bin/fscryptctl -a -r /proc/device-tree/key_dev_specific_512 ]
		then
			cat /proc/device-tree/key_dev_specific_512 | fscryptctl  insert_key
		fi

		echo ">>>>> Mounting /data partition <<<<<"
		mount_data 

		mknod /var/fuse c 10 229
		chmod a+rw /var/fuse
		mkdir -p /var/log /var/run /var/state/dhcp /var/ppp /var/udhcpd /var/zebra /var/siproxd /var/cache /var/tmp /var/samba /var/samba/share /var/samba/homes /var/samba/private /var/samba/locks
		cp  /etc/smb.conf /var/samba/ 2>/dev/null

        for i in $(cat /proc/cmdline) ; do
           case $i in 
              erase_psi)
              echo ">>>> Cleanup PSI was requested <<<<"
              for t in $(ls -A /data) ; do
                  if is_file_to_delete "$t" ; then
                     echo "Removing /data/$t"
                     rm -rf /data/$t
                  fi
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

