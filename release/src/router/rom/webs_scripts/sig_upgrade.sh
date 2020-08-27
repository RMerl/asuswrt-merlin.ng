#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`

wget_options="-q -t 2 -T 30"

nvram set sig_state_upgrade=0 # INITIALIZING
nvram set sig_state_error=0
nvram set sig_state_update=0

touch /tmp/update_url
update_url=`cat /tmp/update_url`
#update_url="http://192.168.123.198"

sig_file=`nvram get SKU`_`nvram get sig_state_info`_un.zip
sig_rsasign=`nvram get SKU`_`nvram get sig_state_info`_rsa`nvram get live_update_rsa_ver`.zip

# get signature zip file
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi

#urlpath=`nvram get sig_state_url`
if [ -z "$IS_BCMHND" ]; then
	echo 3 > /proc/sys/vm/drop_caches
fi

echo "---- sig upgrade start: ----" >> /tmp/sig_upgrade.log

wget_result=0
wget_result2=0
if [ "$update_url" != "" ]; then
	echo "---- wget sig /tmp/update_url ----" >> /tmp/sig_upgrade.log
	wget $wget_options ${update_url}/$sig_file -O /tmp/rule.trf
	wget_result=$?
	echo "---- wget sig, exit code: ${wget_result} ----" >> /tmp/sig_upgrade.log

	wget $wget_options ${update_url}/$sig_rsasign -O /tmp/rsasign.bin
	wget_result2=$?
	echo "---- wget rsa, exit code: ${wget_result2} ----" >> /tmp/sig_upgrade.log
elif [ "$forsq" == "1" ]; then
	echo "---- wget sig sq ----" >> /tmp/sig_upgrade.log
	wget $wget_options http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/$sig_file -O /tmp/rule.trf
	wget_result=$?
	echo "---- wget sig, exit code: ${wget_result} ----" >> /tmp/sig_upgrade.log

	wget $wget_options http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/$sig_rsasign -O /tmp/rsasign.bin
	wget_result2=$?
	echo "---- wget rsa, exit code: ${wget_result2} ----" >> /tmp/sig_upgrade.log
elif [ "$urlpath" == "" ]; then
	echo "---- wget sig Real ----" >> /tmp/sig_upgrade.log
	wget $wget_options http://dlcdnet.asus.com/pub/ASUS/wireless/ASUSWRT/$sig_file -O /tmp/rule.trf
	wget_result=$?
	echo "---- wget sig, exit code: ${wget_result} ----" >> /tmp/sig_upgrade.log

	wget $wget_options http://dlcdnet.asus.com/pub/ASUS/wireless/ASUSWRT/$sig_rsasign -O /tmp/rsasign.bin
	wget_result2=$?
	echo "---- wget rsa, exit code: ${wget_result2} ----" >> /tmp/sig_upgrade.log
else
	echo "---- wget sig URL ----" >> /tmp/sig_upgrade.log
	wget $wget_options $urlpath/$sig_file -O /tmp/rule.trf
	wget_result=$?
	echo "---- wget sig, exit code: ${wget_result} ----" >> /tmp/sig_upgrade.log

	wget $wget_options $urlpath/$sig_rsasign -O /tmp/rsasign.bin
	wget_result2=$?
	echo "---- wget rsa, exit code: ${wget_result2} ----" >> /tmp/sig_upgrade.log
fi


if [ "$wget_result" != "0" ]; then	#download failure
	echo "---- download sig failure ----" >> /tmp/sig_upgrade.log 
	nvram set sig_state_error=1
elif [ "$wget_result2" != "0" ]; then	#download failure
	echo "---- download rsa failure ----" >> /tmp/sig_upgrade.log 
	nvram set sig_state_error=1
else
	nvram set sig_state_upgrade=2	
	echo "---- mv trf OK ----" >> /tmp/sig_upgrade.log

	nvram set bwdpi_rsa_check=0
	rsasign_sig_check /tmp/rule.trf
	sleep 1

	rsasign_check_ret=`nvram get bwdpi_rsa_check`

	if [ "$rsasign_check_ret" == "1" ]; then
		echo "---- sig check OK ----" >> /tmp/sig_upgrade.log
		nvram set sig_update_t=`date +%s`   #set timestamp for download signature and restart_wrs
		if [ -f /jffs/signature/rule.trf ];then
			echo "---- sig rule mv /tmp to /jffs/signature ----" >> /tmp/sig_upgrade.log
			rm /jffs/signature/rule.trf
			mv /tmp/rule.trf /jffs/signature/rule.trf
		else
			echo "---- sig rule mv jffs ----" >> /tmp/sig_upgrade.log
			mkdir /jffs/signature
			mv /tmp/rule.trf /jffs/signature/rule.trf
		fi
		if [ "$1" == "" ];then
			rc rc_service restart_wrs
		else
			echo "do nothing..."	
		fi
	else
		echo "---- sig check error ----" >> /tmp/sig_upgrade.log
		nvram set sig_state_error=3	# wrong sig trf
	fi
fi

rm -f /tmp/rsasign.bin

echo "---- sig upgrade end ----" >> /tmp/sig_upgrade.log
nvram set sig_state_upgrade=1
