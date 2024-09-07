#!/bin/sh

ifnames=`nvram get _acs_restart_ifnames`

for wlif in $ifnames
do
	echo "do acs_cli2 -i $wlif acs_restart"
	acs_cli2 -i $wlif acs_restart
	sleep 1
done
