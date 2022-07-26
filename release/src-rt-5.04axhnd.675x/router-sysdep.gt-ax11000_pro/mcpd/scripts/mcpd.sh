#!/bin/sh

# Start mcpd on system startup.
# Initially start with empty config file, which causes mcpd to run with 
# default settings.  When things get configured, a new config file will be
# written out and run "mcpctl reload".

case "$1" in
	start)
		echo "Starting mcpd..."
		# create empty config file
		# MCPD_CONFIG_FILE ==>"/var/mcpd.conf" in rut_multicast.h and mcpd_config.c
		cat /dev/null > /var/mcpd.conf
		mcpd &
		exit 0
		;;

	*)
		echo "$0: unrecognized or unsupported option $1"
		exit 1
		;;

esac

