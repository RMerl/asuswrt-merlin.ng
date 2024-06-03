#!/bin/sh

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
		# Basically copy the old coredump files to /data/core.last/
		# Set the /proc/sys/kernel/core_pattern to /data/core.new/core_%e
		# Transfer core file from target to host using nc:
		# on target, nc 192.168.1.100 54012 < core_appname
		# on host, receive with nc -p 54012 > core_appname
		# on host, cd targets/<PROFILE>
		# /opt/toolchains/crosstools-arm-gcc-9.2-linux-4.19-glibc-2.30-binutils-2.32/usr/bin/arm-buildroot-linux-gnueabi-gdb ../../userspace/path_to_app
		# (gdb) set solib-absolute-prefix ./fs.install 
		# (gdb) core core_appname
		# (gdb) bt
		if [ ! -z "$ENABLE_APP_COREDUMPS" ]; then
			# No cordump file if the remaining of /data is less than 4M.
			data_left=`df /data | grep /data | (read a b c d e f; echo $d)`
			if [ "$data_left" -gt "4000" ]; then
				CORE_DIR_NEW=/data/core.new
				CORE_DIR_LAST=/data/core.last
				echo "Enabling application coredumps to $CORE_DIR_NEW"
				echo "${CORE_DIR_NEW}/core_%e" > /proc/sys/kernel/core_pattern
				if [ ! -d $CORE_DIR_NEW ]; then
					mkdir $CORE_DIR_NEW
				else
					nfiles=`ls $CORE_DIR_NEW/`
					if [ "$nfiles" != "" ]; then
						echo "Detected coredumps from last boot: $nfiles"
						if [ ! -d $CORE_DIR_LAST ]; then
							mkdir $CORE_DIR_LAST
						else
							rm -f $CORE_DIR_LAST/*
						fi
						mv $CORE_DIR_NEW/* $CORE_DIR_LAST/.
					fi
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
		cp /etc/passwd.static /var/passwd
		cp /etc/group.static /var/group

		echo "Configuring system OK"

		exit 0
		;;

	stop)
		echo "Unconfig system not implemented..."
		exit 1
		;;

	*)
		echo "$0: unrecognized option $1"
		exit 1
		;;

esac

