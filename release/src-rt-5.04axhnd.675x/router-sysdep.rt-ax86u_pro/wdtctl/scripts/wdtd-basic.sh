#!/bin/sh

# /etc/init.d startup script for the "basic" version of
# watchdog timer daemon (wdtd).  The wdtd is started early in the userspace
# startup to keep pinging the watchdog, and keeps running while the system
# is running.  It is never terminated.

case "$1" in
	start)
		echo "Starting watchdog timer daemon (wdtd)..."
		/bin/wdtctl -d -t 30 start
		exit 0
		;;

	*)
		echo "$0: unrecognized or unsupported option $1"
		exit 1
		;;

esac

