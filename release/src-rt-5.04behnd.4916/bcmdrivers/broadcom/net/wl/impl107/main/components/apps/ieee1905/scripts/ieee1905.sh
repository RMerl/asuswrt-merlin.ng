#!/bin/sh

case "$1" in
	start)
		echo "Starting ieee1905..."
		/bin/ieee1905 &
		exit 0
		;;

	stop)
		echo "Stopping ieee1905..."
		echo "Not implemented yet..."
		exit 0
		;;

	*)
		echo "$0: unrecognized option $1"
		exit 1
		;;

esac
