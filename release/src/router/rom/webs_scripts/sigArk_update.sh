#!/bin/sh

wget_options="--ciphers=DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES -q -t 2 -T 30"

nvram set ark_state_update=0 # INITIALIZING
nvram set ark_state_flag=0   # 0: Don't do upgrade  1: Do upgrade
nvram set ark_sig_state=1    # CHECKING_UPDATE
nvram set sig_state_error=0
nvram set ark_sig_unzip=0

# current ark signature information : GS_42512010L_10M
current_ark_ver="$(nvram get ark_sig_ver)"
current_ark_ver="$(echo "$current_ark_ver" | sed 's/\.//g')"

# current ark engine version
current_ark_engine="${current_ark_ver:3:1}"
# current ark attr
current_ark_attr="${current_ark_ver:11:5}"
# current ark attr
current_ark_ver_num="${current_ark_ver:4:7}"

#for SQ test
forsq="$(nvram get apps_sq)"
if [ -z "$forsq" ]; then
	forsq=0
fi

# get signature information
tcode="$(nvram get territory_code)"
if [ "$tcode" == "" ]; then
	territory_type="WW"_"${current_ark_engine}${current_ark_attr}"
else
	territory_type="$tcode"_"${current_ark_engine}${current_ark_attr}"
fi

echo "---- sigArk update start: ----" > /tmp/sigArk_upgrade.log
if [ "$forsq" == "1" ]; then
	echo "---- sigArk update sq normal----" >> /tmp/sigArk_upgrade.log
	wget ${wget_options} "https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/sigArk_update.zip" -O /tmp/sigArk_update.txt
else
	echo "---- sigArk update real normal----" >> /tmp/sigArk_upgrade.log
	wget ${wget_options} "https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/sigArk_update.zip" -O /tmp/sigArk_update.txt
fi
dlinfo="$?"
echo "---- sigArk wget exit : $dlinfo ----" >> /tmp/sigArk_upgrade.log

if [ "$dlinfo" != "0" ]; then
	echo "Download sigArk info Failure" >> /tmp/sigArk_upgrade.log
	nvram set sig_state_error=4
	nvram set ark_sig_unzip=1
	nvram set ark_sig_state=6   # FAILED
else
	echo "Download sigArk info OK" >> /tmp/sigArk_upgrade.log
	# TODO get and parse information WW_4L_10M#2512150
	sig_ver="$(grep "$territory_type" /tmp/sigArk_update.txt | sed 's/.*#//')"
	if [ "$sig_ver" == "" ]; then
		WW_type="WW"_"${current_ark_engine}${current_ark_attr}"
		sig_ver="$(grep "$WW_type" /tmp/sigArk_update.txt | sed 's/.*#//')"
		nvram set Ark_SKU="${WW_type}"
	else
		territory_type="$(echo "$territory_type" | sed 's/\//_/g')"
		nvram set Ark_SKU="$territory_type"
	fi
	echo "$(nvram get Ark_SKU)" >> /tmp/sigArk_upgrade.log
	echo $sig_ver >> /tmp/sigArk_upgrade.log

	sig_ver="$(echo "$sig_ver" | sed 's/\.//g')"
	nvram set sigArk_state_info="${sig_ver}"
	# dont remove tmp: rm -f /tmp/sigArk_update.*
fi

echo "---- current sigArk : $current_ark_ver ----" >> /tmp/sigArk_upgrade.log
logger -t SIG_ARK_UPDATE "current sigArk : $current_ark_ver"
echo "---- current sigArk num : $current_ark_ver_num ----" >> /tmp/sigArk_upgrade.log
logger -t SIG_ARK_UPDATE "current sigArk_num : $current_ark_ver_num"
echo "---- latest sigArk : $sig_ver ----" >> /tmp/sigArk_upgrade.log
logger -t SIG_ARK_UPDATE "latest sigArk : $sig_ver"

if [ "$sig_ver" == "" ]; then
	echo "---- parse no Ark info ---" >> /tmp/sigArk_upgrade.log
	logger -t SIG_ARK_UPDATE "parse no info"
	nvram set sig_state_error=4	# exist no Ark Info
	nvram set ark_sig_unzip=1
	nvram set ark_sig_state=6   # FAILED
else
	if [ -z "$current_ark_ver_num" ] || [ -z "$sig_ver" ]; then
		nvram set ark_sig_unzip=1
		nvram set ark_sig_state=6   # FAILED
	elif [ "$current_ark_ver_num" -lt "$sig_ver" ]; then
		echo "---- < latest sigArk, Do Ark upgrade ----" >> /tmp/sigArk_upgrade.log
		logger -t SIG_ARK_UPDATE "< latest sigArk, Do Ark upgrade"
		nvram set ark_state_flag=1	# Do upgrade
		nvram set ark_sig_state=2   # UPGRADE_REQUIRED
	else
		nvram set ark_sig_unzip=1
		nvram set ark_sig_state=0   # IDLE
	fi
fi

echo "---- sigArk update end ----" >> /tmp/sigArk_upgrade.log
nvram set ark_state_update=1
