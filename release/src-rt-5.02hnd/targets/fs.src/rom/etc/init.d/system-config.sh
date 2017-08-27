#!/bin/sh

case "$1" in
	start)
		echo "Configuring system..."
		# these are some miscellaneous stuff without a good home
		ifconfig lo 127.0.0.1 netmask 255.0.0.0 broadcast 127.255.255.255 up
		echo > /var/udhcpd/udhcpd.leases
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

