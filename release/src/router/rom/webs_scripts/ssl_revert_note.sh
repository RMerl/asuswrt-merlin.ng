#!/bin/sh

betaupg_support=`nvram get rc_support|grep -i betaupg`

wget_options="-q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"
nvram set cfg_note=0

get_preferred_lang=`nvram get preferred_lang`
LANG="$get_preferred_lang"

echo "---- [REVERTFW] Get release note start ----" >> /tmp/webs_upgrade.log

# org fw information
firmver_org=`nvram get firmver_org`
firmver_org=`echo $firmver_org | sed s/'\.'//g;`
buildno_org=`nvram get buildno_org`
extendno_org=`nvram get extendno_org`
if [ "$firmver_org" == "" ] || [ "$buildno_org" == "" ] || [ "$extendno_org" == "" ]; then
	revert_version=0
else
	revert_version=`echo $firmver_org`_`echo $buildno_org`_`echo $extendno_org`
fi

# for beta path
forbeta=0
if [ "$betaupg_support" != "" ]; then
	firmver_org_1st_bit=${firmver_org:0:1}
	echo "---- firmver_org_1st_bit : $firmver_org_1st_bit ----" >> /tmp/webs_upgrade.log

        if [ "$firmver_org_1st_bit" == "9" ]; then
                forbeta=1
        fi
        echo "---- forbeta : $forbeta ----" >> /tmp/webs_upgrade.log
fi

# model information
productid=`nvram get productid`
get_productid=`echo $productid | sed s/+/plus/;`    #replace 'plus' to '+' for one time
odmpid=`nvram get odmpid`
get_odmpid=`echo $odmpid | sed s/+/plus/;`    #replace 'plus' to '+' for one time
odmpid_support=`nvram get webs_state_odm`
odmpid_support=`echo $odmpid_support | sed s/+/plus/;`    #replace 'plus' to '+' for one time
echo "---- get_productid|odmpid_support|get_odmpid : $get_productid|$odmpid_support|$get_odmpid ----" >> /tmp/webs_upgrade.log
if [ "$odmpid_support" == "1" ] || [ "$odmpid_support" == "$get_odmpid" ]; then	# MIS || FRS
	if [ "$get_odmpid" != "" ]; then
		get_productid=$get_odmpid
	fi
fi
echo "---- get_productid : $get_productid ----" >> /tmp/webs_upgrade.log

# release note
releasenote_file="$get_productid"_"$revert_version"_"$LANG"_note.zip
releasenote_file_US="$get_productid"_"$revert_version"_US_note.zip
releasenote_path="/tmp/release_note.txt"


#kill old files
rm -f $releasenote_path

wget_release=""
wget_release2=""
if [ "$revert_version" == "0" ]; then
	echo "---- [REVERTFW] No info for revert fw version ----" >> /tmp/webs_upgrade.log
else

if [ "$betaupg_support" != "" ] && [ "$forbeta" == "1" ]; then
	echo "---- download beta release note ${dl_path_SQ}/$releasenote_file ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ}/$releasenote_file -O $releasenote_path
	wget_release=$?
	echo "---- [REVERTFW] wget pLang release note, exit code: ${wget_release} ----" >> /tmp/webs_upgrade.log
	if [ "$wget_release" != "0" ]; then
		echo "---- download beta release note ${dl_path_SQ}/$releasenote_file_US ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ}/$releasenote_file_US -O $releasenote_path
		wget_release2=$?
		echo "---- [REVERTFW] wget US release note, exit code: ${wget_release2} ----" >> /tmp/webs_upgrade.log
	fi
else
	echo "---- download official release note ${dl_path_file}/$releasenote_file ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_file}/$releasenote_file -O $releasenote_path
	wget_release=$?
	echo "---- [REVERTFW] wget pLang release note, exit code: ${wget_release} ----" >> /tmp/webs_upgrade.log
	if [ "$wget_release" != "0" ]; then
		echo "---- download real release note ${dl_path_file}/$releasenote_file_US ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_file}/$releasenote_file_US -O $releasenote_path
		wget_release2=$?
		echo "---- [REVERTFW] wget US release note, exit code: ${wget_release2} ----" >> /tmp/webs_upgrade.log
	fi
fi

fi	#"$revert_version"

if [ "$revert_version" == "0" ]; then
	echo "---- [REVERTFW] No info for revert fw version ----" >> /tmp/webs_upgrade.log
        nvram set cfg_note=3
        exit 0
elif [ "$wget_release" != "0" ] && [ "$wget_release2" != "0" ]; then
	echo "---- [REVERTFW] download release note failed ----" >> /tmp/webs_upgrade.log
	nvram set cfg_note=3
	exit 0
elif [ ! -s $releasenote_path ]; then
	echo "---- [REVERTFW] no exist release note ----" >> /tmp/webs_upgrade.log
	nvram set cfg_note=3
	exit 0
else
	echo "---- [REVERTFW] Get release note successfully ----" >> /tmp/webs_upgrade.log
fi

echo "---- [REVERTFW] Get release note end. ----" >> /tmp/webs_upgrade.log
nvram set cfg_note=1
