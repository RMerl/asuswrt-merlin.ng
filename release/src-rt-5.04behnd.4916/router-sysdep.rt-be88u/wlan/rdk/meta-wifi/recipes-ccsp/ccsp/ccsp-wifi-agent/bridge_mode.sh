#!/bin/sh

########################### FOR BRIDGE MODE SET UP ##############################

sleep 5

BRIDGE_MODE=`dmcli eRT getv Device.X_CISCO_COM_DeviceControl.LanManagementEntry.1.LanMode | grep value | cut -d ':' -f3 | cut -d ' ' -f2`
PRIVATE_WIFI_2G=`cat /nvram/hostapd0.conf | grep interface= | head -n1 | cut -d '=' -f2`
PRIVATE_WIFI_5G=`cat /nvram/hostapd1.conf | grep interface= | head -n1 | cut -d '=' -f2`
echo "BRIDGE MODE is $BRIDGE_MODE"
if [ "$BRIDGE_MODE" = "bridge-static" ] ; then
	sysevent set lan-stop
	ifconfig $PRIVATE_WIFI_2G down
	ifconfig $PRIVATE_WIFI_5G down 
	ps aux | grep hostapd1 | grep -v grep | awk '{print $1}' | xargs kill -9
	ps aux | grep hostapd0 | grep -v grep | awk '{print $1}' | xargs kill -9
else
	echo "Running in Router Mode"
fi
