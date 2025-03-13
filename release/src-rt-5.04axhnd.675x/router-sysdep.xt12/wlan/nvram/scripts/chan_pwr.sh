#!/bin/sh
# $1: WiFi interface


if [ -z "$1" ]; then
	echo "Usage: $0 [WiFi interface name]"
	exit 0
fi


intf_chan=`wl -i $1 chanspec |awk 'BEGIN{FS=" "}{print $2}'`
#echo $intf_chan

chan_pwr=`wl -i $1 chanspec_txpwr_max |grep "${intf_chan}"`
#echo $chan_pwr

pwr_dB=`echo "${chan_pwr}" |awk 'BEGIN{FS=" "}{print $3}'`
#echo $pwr_dB

only_pwr=`echo "${pwr_dB}" |awk 'BEGIN{FS="("}{print $1}'`
echo $only_pwr

