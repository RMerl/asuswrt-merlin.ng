#!/bin/sh

if [ -f /jffs/scripts/openvpn-event ]
then
	if [ "$(nvram get jffs2_scripts)" = "0" ]
	then
		/usr/bin/logger -t "custom_script" "Found openvpn-event, but custom script execution is disabled!"
		exit 0
	fi
	/usr/bin/logger -t "custom_script" "Running /jffs/scripts/openvpn-event (args: $*)"
	/bin/sh /jffs/scripts/openvpn-event $*
fi

exit 0
