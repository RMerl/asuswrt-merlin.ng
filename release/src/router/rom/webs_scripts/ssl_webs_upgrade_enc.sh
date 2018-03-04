#!/bin/sh

wget_timeout=`nvram get apps_wget_timeout`
#wget_options="-nv -t 2 -T $wget_timeout --dns-timeout=120"
wget_options="-q -t 2 -T $wget_timeout --no-check-certificate"

nvram set webs_state_upgrade=0 # INITIALIZING
nvram set webs_state_error=0

touch /tmp/update_url
update_url=`cat /tmp/update_url`

get_productid=TM-AC1900
#get_productid=`echo $get_productid | sed s/+/plus/;`	#replace 'plus' to '+' for one time

firmware_file=TM-AC1900_`nvram get webs_state_info`_enc.zip

small_fw_update=`nvram show | grep rc_support | grep small_fw`

firmware_path="/tmp/linux.trx"

# get firmware zip file
forsq=`nvram get apps_sq`
urlpath=`nvram get webs_state_url`
echo 3 > /proc/sys/vm/drop_caches
if [ "$update_url" != "" ]; then
	echo "---- wget fw nvram webs_state_url ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${update_url}/$firmware_file -O /tmp/linux_enc.trx
elif [ "$forsq" == "1" ]; then
	echo "---- wget fw sq ----" >> /tmp/webs_upgrade.log
	wget $wget_options https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/$firmware_file -O /tmp/linux_enc.trx
elif [ "$urlpath" == "" ]; then
	echo "---- wget fw Real ----" >> /tmp/webs_upgrade.log
	wget $wget_options https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT/$firmware_file -O /tmp/linux_enc.trx
else
	echo "---- wget fw URL ----" >> /tmp/webs_upgrade.log
	wget $wget_options $urlpath/$firmware_file -O /tmp/linux_enc.trx
fi	

if [ "$?" != "0" ]; then	#download failure
	nvram set webs_state_error=1
else
	fwlength=`nvram get webs_state_fwlength`
	fwcrc=`nvram get webs_state_fwcrc`
	dl_length=`ls -nl /tmp/linux_enc.trx | awk '{print $5}'`
	firmware_enc_crc /tmp/linux_enc.trx
	sleep 3
	dl_crc=`nvram get fw_enc_crc`
	if [ "$dl_length" != "$fwlength"  ]; then
		echo "download fw size inconsistent. $dl_length : $fwlength" >> /tmp/webs_upgrade.log
		logger -t webs_upgrade download fw size inconsistent. $dl_length : $fwlength
	elif [ "$dl_crc" != "$fwcrc" ]; then
		echo "download fw crc inconsistent. $dl_crc : $fwcrc" >> /tmp/webs_upgrade.log
		logger -t webs_upgrade download fw crc inconsistent. $dl_crc : $fwcrc
	else
		nvram set webs_state_upgrade=2	
		echo "---- mv trx OK ----" >> /tmp/webs_upgrade.log
		nvram set firmware_check=0
		firmware_check $firmware_path
		sleep 6

		firmware_check_ret=`nvram get firmware_check`

		if [ "$firmware_check_ret" == "1" ]; then
			echo "---- fw check OK ----" >> /tmp/webs_upgrade.log
			logger -t webs_upgrade fw check OK
			/sbin/ejusb -1 0
			rc rc_service restart_upgrade
		else
			echo "---- fw check error ----" >> /tmp/webs_upgrade.log
			logger -t webs_upgrade fw check error
			nvram set webs_state_error=3	# wrong fw
		fi
	fi
fi

nvram set webs_state_upgrade=1
