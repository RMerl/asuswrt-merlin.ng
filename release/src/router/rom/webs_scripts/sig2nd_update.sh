#!/bin/sh

wget_options="--ciphers=DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES -q -t 2 -T 30 --no-check-certificate"

nvram set sig_state_update=0 # INITIALIZING
nvram set sig_state_flag=0   # 0: Don't do upgrade  1: Do upgrade	
nvram set sig_state_error=0
#nvram set sig_state_url=""

# current signature information
current_sig_ver=`nvram get bwdpi_sig_ver`
current_sig_ver=`echo $current_sig_ver | sed s/'\.'//g;`

#get HNS / FULL / PARTIAL / Lite signature: HNS/FULL/PART/WRS @update_sig_type() iqos.c
sig_type=`nvram get sig_type`
if [ "$sig_type" == "" ]; then
	sig_type="FULL";
fi

#for SQ test
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi

# get signature information
tcode=`nvram get territory_code`
if [ "$tcode" == "" ]; then
	territory_type="WW"_"$sig_type"
else
	territory_type="$tcode"_"$sig_type"
fi

echo "---- sig update start: ----" > /tmp/sig_upgrade.log
if [ "$forsq" == "1" ]; then
	echo "---- sig update sq normal----" >> /tmp/sig_upgrade.log
	wget ${wget_options} https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/sig2nd_update.zip -O /tmp/sig_update.txt
else
	echo "---- sig update real normal----" >> /tmp/sig_upgrade.log
	wget ${wget_options} https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/sig2nd_update.zip -O /tmp/sig_update.txt
fi
dlinfo="$?"
echo "---- sig wget exit : $dlinfo ----" >> /tmp/sig_upgrade.log

if [ "$dlinfo" != "0" ]; then
	echo "Download sig info Failure" >> /tmp/sig_upgrade.log
	nvram set sig_state_error=1
else
	echo "Download sig info OK" >> /tmp/sig_upgrade.log
	# TODO get and parse information
	sig_ver=`grep $territory_type /tmp/sig_update.txt | sed s/.*#//;`
	if [ "$sig_ver" == "" ]; then
		WW_type="WW"_$sig_type
		sig_ver=`grep $WW_type /tmp/sig_update.txt | sed s/.*#//;`
		nvram set SKU="WW"_"$sig_type"
	else
		territory_type=`echo $territory_type | sed s/'\/'/'\_'/g;`
		nvram set SKU="$territory_type"
	fi
	echo `nvram get SKU` >> /tmp/sig_upgrade.log
	echo $sig_ver >> /tmp/sig_upgrade.log
	
	sig_ver=`echo $sig_ver | sed s/'\.'//g;`
	nvram set sig_state_info=${sig_ver}
	rm -f /tmp/sig_update.*
fi

echo "---- current sig : $current_sig_ver ----" >> /tmp/sig_upgrade.log
logger -t SIG_UPDATE "current sig : $current_sig_ver"
echo "---- latest sig : $sig_ver ----" >> /tmp/sig_upgrade.log
logger -t SIG_UPDATE "latest sig : $sig_ver"

if [ "$sig_ver" == "" ]; then
	echo "---- parse no info ---" >> /tmp/sig_upgrade.log
	logger -t SIG_UPDATE "parse no info"
	nvram set sig_state_error=1	# exist no Info
else
	if [ "$current_sig_ver" -lt "$sig_ver" ]; then
		echo "---- < sig_ver, Do upgrade ----" >> /tmp/sig_upgrade.log
		logger -t SIG_UPDATE "< sig_ver, Do upgrade"
		nvram set sig_state_flag=1	# Do upgrade
	fi	
fi

echo "---- sig update end ----" >> /tmp/sig_upgrade.log
nvram set sig_state_update=1
