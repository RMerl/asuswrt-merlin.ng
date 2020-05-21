#!/bin/sh

wget_timeout=`nvram get apps_wget_timeout`
#wget_options="-nv -t 2 -T $wget_timeout --dns-timeout=120"
wget_options="-q -t 2 -T $wget_timeout --no-check-certificate"

nvram set webs_state_update=0 # INITIALIZING
nvram set webs_state_flag=0   # 0: Don't do upgrade  1: Do upgrade	
nvram set webs_state_error=0
nvram set webs_state_url=""
nvram set webs_state_fwlength=0
nvram set webs_state_fwcrc=0

# current firmware information
current_firm=`nvram get firmver`
current_firm=`echo $current_firm | sed s/'\.'//g;`
current_buildno=`nvram get buildno`
current_extendno=`nvram get extendno`
current_extendno=`echo $current_extendno | sed s/-g.*//;`

# get firmware information
forsq=`nvram get apps_sq`
model=TM-AC1900

if [ "$forsq" == "1" ]; then
	echo "---- update sq tmo ----" >> /tmp/webs_upgrade.log
	wget $wget_options https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/wlan_update_tmo_enc.zip -O /tmp/wlan_update.txt
else
	echo "---- update real tmo ----" >> /tmp/webs_upgrade.log
	wget $wget_options https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/wlan_update_tmo_enc.zip -O /tmp/wlan_update.txt
fi	

if [ "$?" != "0" ]; then
	echo "---- download INFO failed ----" >> /tmp/webs_upgrade.log
	nvram set webs_state_error=1
else
	# TODO get and parse information
	firmver=`grep $model /tmp/wlan_update.txt | sed s/.*#FW//;`
	firmver=`echo $firmver | sed s/#.*//;`
	buildno=`echo $firmver | sed s/....//;`
	firmver=`echo $firmver | sed s/$buildno$//;`
	extendno=`grep $model /tmp/wlan_update.txt | sed s/.*#EXT//;`
	extendno=`echo $extendno | sed s/#.*//;`
	lextendno=`echo $extendno | sed s/-g.*//;`
	nvram set webs_state_info=${firmver}_${buildno}_${extendno}
	urlpath=`grep $model /tmp/wlan_update.txt | sed s/.*#URL//;`	
	urlpath=`echo $urlpath | sed s/#.*//;`	
	nvram set webs_state_url=${urlpath}
	fwlength=`grep $model /tmp/wlan_update.txt | sed s/.*#LENGTH//;`
	fwlength=`echo $fwlength | sed s/#.*//;`
	nvram set webs_state_fwlength=${fwlength}
	fwcrc=`grep $model /tmp/wlan_update.txt | sed s/.*#CRC//;`
	fwcrc=`echo $fwcrc | sed s/#.*//;`
	nvram set webs_state_fwcrc=${fwcrc}
	stat_file=`nvram get productid`_`nvram get webs_state_info`_stat.zip  #to test quantity of downloading
	rm -f /tmp/wlan_update.*
fi

#to test quantity of downloading
#if [ "$forsq" == "1" ]; then
#	echo "---- update sq quantity test----" >> /tmp/webs_upgrade.log
#	wget -q http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/$stat_file -O /tmp/wlan_stat.txt
#else
#	echo "---- update real quantity test----" >> /tmp/webs_upgrade.log
#	wget -q http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/$stat_file -O /tmp/wlan_stat.txt
#fi

echo "---- cur: $current_firm $current_buildno $current_extendno ----" >> /tmp/webs_upgrade.log
if [ "$firmver" == "" ] || [ "$buildno" == "" ] || [ "$lextendno" == "" ]; then
	echo "---- Not found exact model info ----" >> /tmp/webs_upgrade.log
	nvram set webs_state_error=1	# exist no Info
else
	echo "---- tmo: $firmver $buildno $lextendno ----" >> /tmp/webs_upgrade.log
	logger -t webs_update tmo
	nvram set webs_state_flag=1	# Do upgrade
fi

nvram set webs_state_update=1
