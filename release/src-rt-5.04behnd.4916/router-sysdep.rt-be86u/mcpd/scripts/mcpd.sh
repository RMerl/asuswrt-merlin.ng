#!/bin/sh

# Start mcpd on system startup.
# Initially start with empty config file, which causes mcpd to run with 
# default settings.  When things get configured, a new config file will be
# written out and run "mcpctl reload".

. /etc/init.d/startup_ctl.sh

case "$1" in
	start)
		start_up mcpd
		{
		  # create empty config file
		  # MCPD_CONFIG_FILE ==>"/var/mcpd.conf" in rut_multicast.h and mcpd_config.c
		  trap_err
		  cat /dev/null > /var/mcpd.conf
		  mcpd &
		  start_done
		  exit 0
		}&
		;;

	*)
		echo "$0: unrecognized or unsupported option $1"
		exit 1
		;;

esac

