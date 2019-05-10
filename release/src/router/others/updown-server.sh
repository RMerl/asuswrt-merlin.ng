#!/bin/sh

if [ -f /jffs/scripts/openvpn-event ]
then
	/usr/bin/logger -t "custom_script" "Running /jffs/scripts/openvpn-event (args: $*)"
	/bin/sh /jffs/scripts/openvpn-event $*
fi

exit 0
