#!/bin/sh

trap "" 2

#The following will be populated by buildFS during the make process:
KERNELVER=_set_by_buildFS_

if [ ! -d /lib/modules/$KERNELVER/extra ]; then
	echo "ERROR: gre-tunnel-drivers.sh: /lib/modules/$KERNELVER/extra does not exist" 1>&2
fi

case "$1" in
	start)
		if [ -e /lib/modules/$KERNELVER/kernel/net/ipv4/gre.ko ]; then
			echo "Loading GRE kernel modules..."
			insmod /lib/modules/$KERNELVER/kernel/net/ipv4/gre.ko
			# assume if gre.ko is present, ip_gre.ko is also present
			# if not, we will get a nice error message
			insmod /lib/modules/$KERNELVER/kernel/net/ipv4/ip_gre.ko
		fi

		exit 0
		;;

	stop)
		echo "removing GRE kernel modules not implemented yet..."
		exit 1
		;;

	*)
		echo "bcmbasedrivers: unrecognized option $1"
		exit 1
		;;

esac

