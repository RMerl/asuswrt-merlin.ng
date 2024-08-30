#!/bin/sh

# /etc/init.d startup script for the Firmware Upgrade Watchdog Timer feature.
# The watchdog timer is only pinged once here, early in system startup.
# When the system has fully booted, then the wdtd is started by the
# userspace code.

case "$1" in
	start)
		echo "Ping watchdog to buy more time for system to fully boot..."
		/bin/wdtctl ping
		exit 0
		;;

	*)
		echo "$0: unrecognized or unsupported option $1"
		exit 1
		;;

esac

