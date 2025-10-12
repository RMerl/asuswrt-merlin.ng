#!/bin/sh

retry=0
wifi_driver_initialized=`iw wl0 info|grep wiphy| wc -l`
echo "wifi_driver_initialized: $wifi_driver_initialized"

while [[ $wifi_driver_initialized -eq 0 && $retry -lt 4 ]];
do
	wifi_driver_initialized=`iw wl0 info|grep wiphy| wc -l`
	echo "!!!!!wifi_driver_initialized: $wifi_driver_initialized, retry:$retry"
	sleep 5
	retry=$((retry+1))
done

if [ $wifi_driver_initialized != 0 ]; then
	echo "run wifi_rdk_initd"
	wifi_rdk_initd
	echo "run wifi_rdk_initd DONE!!!!"
else
	echo "Wifi driver is not initialized!"
fi
