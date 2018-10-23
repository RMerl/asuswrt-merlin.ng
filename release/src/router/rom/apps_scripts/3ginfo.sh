#!/bin/sh

kernel_version=`uname -r`
ver_1st=`echo -n $kernel_version |awk 'BEGIN{FS="."}{print $1}'`
ver_2nd=`echo -n $kernel_version |awk 'BEGIN{FS="."}{print $2}'`


echo ">"
if [ "$ver_1st" -ge "4" ]; then
	cat /sys/kernel/debug/usb/devices
elif [ "$ver_1st" -ge "3" ] && [ "$ver_2nd" -ge "2" ]; then
	cat /sys/kernel/debug/usb/devices
else
	cat /proc/bus/usb/devices
fi
echo ">"
lsmod
echo ">"
ifconfig
echo ">"
mount
echo ">"
cat /etc/g3.conf.1
echo ">"
cat /etc/g3.conf.2
echo ">"
cat /tmp/ppp/peers/3g
echo ">"
ls /dev/tty*
echo ">"
nvram get firmver
echo ">"
nvram get buildno
echo ">"
nvram show |grep extendno
echo ">"
echo "dualwan nvram:>"
nvram show |grep ^wans_
echo ">"
echo "IPv6 service:>"
nvram get ipv6_service
echo ">"
echo "link state:>"
nvram show |grep ^link_
echo ">"
echo "wan state:>"
nvram show |grep state |grep wan[01]_
echo ">"
echo "usb path nvram:>"
nvram show |grep ^usb_path1
nvram show |grep ^usb_path2
nvram show |grep ^usb_path3
echo ">"
echo "modem nvram:>"
nvram get Dev3G
echo ">"
nvram show |grep ^modem_ |grep -v "modem_pincode="
echo ">"
echo "modem state:>"
nvram show |grep g3state
echo ">"
nvram show |grep g3err

for modem_unit in 0 1; do
	if [ "$modem_unit" -eq "0" ]; then
		prefix=usb_modem_
	else
		prefix=usb_modem${modem_unit}_
	fi

	act_path=`nvram get ${prefix}act_path`
	if [ -z "$act_path" ]; then
		continue
	fi

	echo ">"
	echo "modem(${modem_unit}) act state:>"
	str=`nvram get ${prefix}act_path`
	echo "${prefix}act_path=$str"
	str=`nvram get ${prefix}act_type`
	echo "${prefix}act_type=$str"
	str=`nvram get ${prefix}act_dev`
	echo "${prefix}act_dev=$str"
	str=`nvram get ${prefix}act_int`
	echo "${prefix}act_int=$str"
	str=`nvram get ${prefix}act_bulk`
	echo "${prefix}act_bulk=$str"
	str=`nvram get ${prefix}act_vid`
	echo "${prefix}act_vid=$str"
	str=`nvram get ${prefix}act_pid`
	echo "${prefix}act_pid=$str"
	str=`nvram get ${prefix}act_sim`
	echo "${prefix}act_sim=$str"
	str=`nvram get ${prefix}act_simdetect`
	echo "${prefix}act_simdetect=$str"
	str=`nvram get ${prefix}act_signal`
	echo "${prefix}act_signal=$str"
	str=`nvram get ${prefix}act_lac`
	echo "${prefix}act_lac=$str"
	str=`nvram get ${prefix}act_rsrq`
	echo "${prefix}act_rsrq=$str"
	str=`nvram get ${prefix}act_rsrp`
	echo "${prefix}act_rsrp=$str"
	str=`nvram get ${prefix}act_rssi`
	echo "${prefix}act_rssi=$str"
	str=`nvram get ${prefix}act_sinr`
	echo "${prefix}act_sinr=$str"
	str=`nvram get ${prefix}act_band`
	echo "${prefix}act_band=$str"
	str=`nvram get ${prefix}act_operation`
	echo "${prefix}act_operation=$str"
	str=`nvram get ${prefix}act_provider`
	echo "${prefix}act_provider=$str"
	str=`nvram get ${prefix}act_imsi |cut -c '1-6'`
	echo "${prefix}act_imsi=$str"
	str=`nvram get ${prefix}act_tx`
	echo "${prefix}act_tx=$str"
	str=`nvram get ${prefix}act_rx`
	echo "${prefix}act_rx=$str"
	str=`nvram get ${prefix}act_swver`
	echo "${prefix}act_swver=$str"
	str=`nvram get ${prefix}act_hwver`
	echo "${prefix}act_hwver=$str"
	str=`nvram get ${prefix}act_scanning`
	echo "${prefix}act_scanning=$str"
	str=`nvram get ${prefix}act_auth`
	echo "${prefix}act_auth=$str"
	str=`nvram get ${prefix}act_auth_pin`
	echo "${prefix}act_auth_pin=$str"
	str=`nvram get ${prefix}act_auth_puk`
	echo "${prefix}act_auth_puk=$str"
	str=`nvram get ${prefix}act_startsec`
	echo "${prefix}act_startsec=$str"
	str=`nvram get ${prefix}act_ip`
	echo "${prefix}act_ip=$str"
	str=`nvram get ${prefix}act_ipv6`
	echo "${prefix}act_ipv6=$str"
	echo ">"
	echo "modem autoapn:>"
	nvram show |grep ^${prefix}auto
done

echo ">"
echo "real ip detect:>"
nvram show |grep "_ipaddr=" |grep wan[01]_
nvram show |grep real |grep wan[01]_
echo ">"
echo "resolv.conf >"
cat /etc/resolv.conf
echo ">"
echo "udhcpd.conf >"
cat /tmp/udhcpd.conf
echo ">"
echo "show dns nvram >"
nvram show |grep dns |grep wan[01]
echo ">"
echo "syslog>"
cat /tmp/syslog.log |tail -n 50
echo ">"
echo "usblog>"
cat /tmp/usb.log
echo ">"
echo "ps>"
ps

