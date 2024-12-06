#!/bin/sh

KERNELVER=_set_by_buildFS_
CORE_DIR_NEW=/data/core.new
CORE_DIR_LAST=/data/core.last

# Re-assemble a reverse oops dump
process_reverse_oops_dump()
{
	:>$1_temp
	:>$1_final
	while IFS= read -r line; do     
		chunk=`echo "$line" | grep -l "$2"`
		if [ -z "$chunk" ]; then
			if [ -f "$1_chunk" ]; then
				echo "$line" >> $1_chunk
			fi
		else
			if [ -f "$1_chunk" ]; then
				cat $1_chunk $1_temp >| $1_final
				cp $1_final $1_temp;
			fi
			:>$1_chunk;
		fi;
	done < $1
	\rm $1_chunk $1_temp
	mv $1_final $1
}

# Kernel crash log config
kernel_oops_config()
{
	ROOTFS_DEV=`/rom/etc/get_rootfs_dev.sh`
	MTDOOPS_PART=/proc/environment/mtdoops
	# Max size(bytes) limit before compression. 131072 = 128KB
	OOPSLOG_MAX_SIZE=131072

	# NAND
	if [ -e $MTDOOPS_PART ]; then
		mtdoops=$(cat $MTDOOPS_PART)
		rec_size=65536
		log_shift=$(gunzip -c /proc/config.gz | grep CONFIG_LOG_BUF_SHIFT | cut -d '=' -f 2)
		rec_size=$((2**$log_shift))
		mtdsize=$(cat /proc/mtd  | grep $mtdoops | cut -d ' ' -f2)
		mtdnum=$(cat /proc/mtd  | grep $mtdoops | cut -d ':' -f1)
		if [ "$mtdsize" != "" ]; then   
			mtdsize=$((0x${mtdsize}))
			if [ $rec_size -gt $mtdsize ]; then
				rec_size=$mtdsize
			fi

			# Header check
			# linux kernel v4.19, #define MTDOOPS_KERNMSG_MAGIC 0x5d005d00
			# linux kernel v6.5.3, #define MTDOOPS_KERNMSG_MAGIC_v1 0x5d005d00, #define MTDOOPS_KERNMSG_MAGIC_v2 0x5d005e00
			mtd_debug read /dev/$mtdnum 0x0 8 /tmp/kernel_crash_log.header >& /dev/null
			mtdoops_kernmsg_magic=$(xxd -g4 -s4 -p /tmp/kernel_crash_log.header)
			if [ "$mtdoops_kernmsg_magic" == "005d005d" -o "$mtdoops_kernmsg_magic" == "005e005d" ]; then
				mtd_debug read /dev/$mtdnum 0x0 $rec_size /tmp/kernel_crash_log.bin >& /dev/null
				# Erase mtdoops partition with 0xff
				mtd_debug erase /dev/$mtdnum 0x0 $mtdsize >& /dev/null
			fi

			# Note: please load mtdoops driver after reading/erasing old oops in above.
			# This ensures mtdoops driver always writes new oops starting at offset 0x0.
			echo "load mtdoops driver: mtd partition $mtdoops record size $rec_size"
			insmod /lib/modules/$KERNELVER/kernel/drivers/mtd/mtdoops.ko mtddev=$mtdoops record_size=$rec_size
		fi

	# MMC
	elif TEMP=`echo $ROOTFS_DEV | grep mmcblk`; then
#		mmcoops_offset=$(xxd -g4 /proc/device-tree/periph/mmcoops/start-offset | cut -d ' ' -f2)
#		mmcoops_size=$(xxd -g4 /proc/device-tree/periph/mmcoops/size | cut -d ' ' -f2)
		mmcoops_offset=$(hexdump -ve '/1 "%02x"' /proc/device-tree/periph/mmcoops/start-offset)
		mmcoops_size=$(hexdump -ve '/1 "%02x"' /proc/device-tree/periph/mmcoops/size)

		if [ "$mmcoops_offset" != "" -a "$mmcoops_size" != "" ]; then
			mmcoops_offset=$((0x${mmcoops_offset}))
			mmcoops_size=$((0x${mmcoops_size}))

			insmod /lib/modules/$KERNELVER/kernel/drivers/mmc/core/oops_mmc.ko mmcdev=/dev/mmcblk0 partition_start_block=$mmcoops_offset partition_size=$(($mmcoops_size*512)) mmc_blksz=512 dump_to_console=n dump_file_path=/var/kernel_crash_log.bin
		fi

	# VFLASH
	elif TEMP=`echo $ROOTFS_DEV | grep 'flash\-'`; then
		vfoops_driver=/lib/modules/$KERNELVER/extra/vfbio_oops.ko
		if [ -f $vfoops_driver ]; then
			vfoops_lun="crashdump"
			# vfoops LUN size(bytes). 262144 = 256KB
			vfoops_size=262144
			if [ ! -e /dev/flash-$vfoops_lun ]; then
				vfbio_lvm create -n $vfoops_lun -m rw -s $vfoops_size
			fi
			rec_size=65536
			log_shift=$(gunzip -c /proc/config.gz | grep CONFIG_LOG_BUF_SHIFT | cut -d '=' -f 2)
			rec_size=$((2**$log_shift))
			if [ $rec_size -gt $vfoops_size ]; then
				rec_size=$vfoops_size
			fi
			block_size=`vfbio_lvm info -n $vfoops_lun | grep "Block size" | (read a b c d e; echo $d)`
			rw_blocks=$(($rec_size/$block_size))
			# insmod vfoops_driver will dump previous crash logs(if present) to dump_file_path and erase 
			# the vfoops_lun. Here only dump panics, because /proc/sys/kernel/panic_on_oops is set.
			insmod $vfoops_driver blkdev=/dev/flash-$vfoops_lun \
				dump_file_path=/tmp/kernel_crash_log.bin rw_blocks=$rw_blocks dump_panic=1
		fi

	# Other
	else
		echo "Skip kernel oops recorder config! FLASH type unsupported."
	fi

	# Move old crash log file
	if [ -f $CORE_DIR_NEW/kernel_crash_log.tar.gz ]; then
		mv $CORE_DIR_NEW/kernel_crash_log.tar.gz $CORE_DIR_LAST/
	fi

	# Save new crash log file
	if [ -f /var/kernel_crash_log.bin ]; then
		# Remove the 0x00 tail from the bin
		tr -d '\000' < /var/kernel_crash_log.bin > /var/kernel_crash_log.bin2
		# Remove the 0xff tail from the bin
		tr -d '\377' < /var/kernel_crash_log.bin2 > /var/kernel_crash_log.txt
		oops_log_size=$(ls -l /var/kernel_crash_log.txt  | awk '{print $5}')
		# If the size exceeds the limit, truncate the tail(valuable) part
		if [ $oops_log_size -gt $OOPSLOG_MAX_SIZE ]; then
			mv /var/kernel_crash_log.txt /var/kernel_crash_log.big
			dd if=/var/kernel_crash_log.big of=/var/kernel_crash_log.txt bs=1 skip=$(($oops_log_size-$OOPSLOG_MAX_SIZE)) >& /dev/null
		fi
		# Check if its a reverse dump. 4.19 eMMC oops driver may use a
		# reverse dump to avoid loss of key oops traces due to eMMC device
		# not being able to fully flush its internal cache due to reset pin 
		# assertion. A reverse oops dump allows the newest traces to be written
		# to the eMMC first, increasing chances of them getting comitted
		if TEMP=`cat /var/kernel_crash_log.txt | grep REV_OOPS`; then
			# Re-assemble reverse dump into proper chronological order
			echo "Reverse oops trace detected!"
			process_reverse_oops_dump /var/kernel_crash_log.txt "REV_OOPS"
		fi

		tar -czf $CORE_DIR_NEW/kernel_crash_log.tar.gz -C /var kernel_crash_log.txt
	fi

	# Remove tmp files
	rm -f /var/kernel_crash_log.*
}

# Application coredump config
coredump_config()
{
	if [ ! -z "$ENABLE_APP_COREDUMPS" ]; then
		# No coredump file if the remaining of /data is less than 4M.
		data_left=`df /data | grep /data | (read a b c d e f; echo $d)`
		if [ "$data_left" -gt "4000" ]; then
			echo "Enabling application coredumps to $CORE_DIR_NEW"
			if [ ! -z "$BUILD_MINICOREDUMPER" ]; then
				COREDUMP_APP=MINI
			else
				COREDUMP_APP=DEFAULT
			fi
			echo "| /rom/etc/init.d/coredump.sh $CORE_DIR_NEW $CORE_DIR_LAST $COREDUMP_APP %E{%e} %P %u %g %s %t %h" > /proc/sys/kernel/core_pattern
			nfiles=`ls $CORE_DIR_NEW/core_* 2> /dev/null`
			if [ "$nfiles" != "" ]; then
				echo "Detected coredumps from last boot: $nfiles"
				mv $CORE_DIR_NEW/core_* $CORE_DIR_LAST/
			fi
		else
			echo "Not enough space in /data for coredumps, have ${data_left}KB, need 4000KB"
			# Setting ulimit -c 0 here has no effect.  If the coredump
			# feature is enabled, busybox/init.c will always set
			# ulimit -c to unlimited for all processes it spawns.  (see targets/buildFS
			# for more details.)  So the only way we can prevent coredumps here
			# is to redirect coredumps to /dev/null.  (This may be overly
			# paranoid since the default core_pattern is just core, which
			# which is probably on a read-only filesystem.  But this logic
			# will protect the system even if the current directory of the
			# crashing app is on a read-write filesystem.)
			echo "/dev/null/core" > /proc/sys/kernel/core_pattern
		fi
	fi
}

crash_log_config()
{
	mkdir -p $CORE_DIR_NEW
	mkdir -p $CORE_DIR_LAST

	# Make sure image version matches crash logs.
	if [ "$(cat $CORE_DIR_NEW/image_version 2> /dev/null)" != "$(cat $CORE_DIR_LAST/image_version 2> /dev/null)" ]; then
		# Wipe out all old records if image_version changed
		rm -rf $CORE_DIR_LAST/*
		cp $CORE_DIR_NEW/image_version $CORE_DIR_LAST/image_version
	fi
	if [ "$(cat /rom/etc/image_version 2> /dev/null)" != "$(cat $CORE_DIR_NEW/image_version 2> /dev/null)" ]; then
		cp /rom/etc/image_version $CORE_DIR_NEW/image_version
	fi

	kernel_oops_config
	coredump_config
}

case "$1" in
	start)
		echo "Configuring system..."
		# these are some miscellaneous stuff without a good home
		ifconfig lo 127.0.0.1 netmask 255.0.0.0 broadcast 127.255.255.255 up
		echo 1 > /proc/sys/kernel/print-fatal-signals

		# Compile-time settings
		source /rom/etc/build_profile
		if [ -z "$RDK_BUILD" ]; then
			echo > /var/udhcpd/udhcpd.leases
		fi

		# Crash log settings
		crash_log_config &

		if [ -n "$BRCM_SCHED_RT_RUNTIME" ]; then
			echo $BRCM_SCHED_RT_RUNTIME > /proc/sys/kernel/sched_rt_runtime_us
		fi
		if [ -n "$BRCM_SCHED_RT_PERIOD" ]; then
			echo $BRCM_SCHED_RT_PERIOD > /proc/sys/kernel/sched_rt_period_us
		fi
		echo 0 > /proc/sys/vm/swappiness

		# set tcp_be_liberal when acceleartion is enabled
		if [ -e /proc/sys/net/netfilter/nf_conntrack_tcp_be_liberal ]; then
			echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_be_liberal
		fi

		# The iptables will automatically call modprobe to install the related kmods.
		# It might not install certain modules with correct parameters.
		# Configure the /proc/sys/kernel/modprobe to an invalid path such the iptables won't install the kmods.
		# Please reconfigure the /proc/sys/kernel/modprobe to /sbin/modprobe if you want to run modprobe.
		echo "/sbin/modprobeX" > /proc/sys/kernel/modprobe

		# enlarge min_free_kbytes for the OOM issue happens during wifi usb-stress testing
		sysctl -qw vm.min_free_kbytes=16384

		# Copy the static versions of /etc/passwd and /etc/group to the runtime location (/var).
		# (This is needed by ubusd early during bootup).
		# The CMS/BDK entries in both files (admin user and root group) will be overwritten by
		# CMS/BDK when it fully starts up.  But all other entries will be preserved.
		cp /rom/etc/passwd.static /var/passwd
		cp /rom/etc/group.static /var/group

		echo "Configuring system OK"

		exit 0
		;;

	stop)
		# To check if a reason(/tmp/reboot_reason) has been written before system call
		# like reboot via bcmUtl_loggedBusybox_reboot.
		# If not, write "Unknown" to /tmp/reboot_reason.
		# The reason needs to be saved on flash(/data/reboot_reason).
		# Compare the content to see if a write operation is really needed.
		REBOOT_REASON_FILE=/tmp/reboot_reason
		LAST_REBOOT_REASON_FILE=/data/reboot_reason
		if [ ! -e $REBOOT_REASON_FILE ]; then
			echo "Unknown" > $REBOOT_REASON_FILE
		fi
		if [ "$(cat $REBOOT_REASON_FILE)" != "$(cat $LAST_REBOOT_REASON_FILE)" ]; then
			cp $REBOOT_REASON_FILE $LAST_REBOOT_REASON_FILE
		fi
		exit 0
		;;

	*)
		echo "$0: unrecognized option $1"
		exit 1
		;;

esac

