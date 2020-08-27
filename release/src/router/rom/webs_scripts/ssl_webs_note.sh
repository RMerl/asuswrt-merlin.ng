#!/bin/sh

wget_options="-q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_info="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

nvram set cfg_note=0
get_preferred_lang=`nvram get preferred_lang`
LANG="$get_preferred_lang"

# get release note
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi

echo "---- Get amas release note start ----" >> /tmp/webs_upgrade.log
if [ $# != 2 ]; then
	echo "---- no model name or version ----" >> /tmp/webs_upgrade.log
	nvram set cfg_note=2
	exit 0
fi

new_firm=`echo $2 | sed s/'\.'/_/4 | sed s/'\.'//g;`
echo "---- $1 $new_firm ----" >> /tmp/webs_upgrade.log
releasenote_file=$1_"$new_firm"_"$LANG"_note.zip
releasenote_file_US=$1_"$new_firm"_US_note.zip
releasenote_path="/tmp/release_note.txt"

wget_release=""
wget_release2=""
if [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	echo "---- download SQ beta_user release note ${dl_path_SQ_beta}${forsq}/$releasenote_file ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ_beta}${forsq}/$releasenote_file -O $releasenote_path
	wget_release=$?
	echo "---- [LiveUpdate] wget pLang release note, exit code: ${wget_release} ----" >> /tmp/webs_upgrade.log
	if [ "$wget_release" != "0" ]; then
		echo "---- download SQ beta_user release note ${dl_path_SQ_beta}${forsq}/$releasenote_file_US ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ_beta}${forsq}/$releasenote_file_US -O $releasenote_path
		wget_release2=$?
		echo "---- [LiveUpdate] wget US release note, exit code: ${wget_release2} ----" >> /tmp/webs_upgrade.log
	fi
elif [ "$forsq" == "1" ]; then
	echo "---- download SQ release note ${dl_path_SQ}/$releasenote_file ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ}/$releasenote_file -O $releasenote_path
	wget_release=$?
	echo "---- [LiveUpdate] wget pLang release note, exit code: ${wget_release} ----" >> /tmp/webs_upgrade.log
	if [ "$wget_release" != "0" ]; then
		echo "---- download SQ release note ${dl_path_SQ}/$releasenote_file_US ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ}/$releasenote_file_US -O $releasenote_path
		wget_release2=$?
		echo "---- [LiveUpdate] wget US release note, exit code: ${wget_release2} ----" >> /tmp/webs_upgrade.log
	fi
else
	echo "---- download real release note ${dl_path_file} for $1 ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_file}/$releasenote_file -O $releasenote_path
	wget_release=$?
	echo "---- [LiveUpdate] wget pLang release note, exit code: ${wget_release} ----" >> /tmp/webs_upgrade.log
	if [ "$wget_release" != "0" ]; then
		echo "---- download real release note ${dl_path_file}/$releasenote_file_US ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_file}/$releasenote_file_US -O $releasenote_path
		wget_release2=$?
		echo "---- [LiveUpdate] wget US release note, exit code: ${wget_release2} ----" >> /tmp/webs_upgrade.log
	fi
fi

if [ "$wget_release" != "0" ] && [ "$wget_release2" != "0" ]; then
	echo "---- download release note failed ----" >> /tmp/webs_upgrade.log
	nvram set cfg_note=3
	exit 0
elif [ ! -s $releasenote_path ]; then
	echo "---- no exist release note ----" >> /tmp/webs_upgrade.log
	nvram set cfg_note=3
	exit 0
else
	echo "---- Get release note successfully ----" >> /tmp/webs_upgrade.log
fi

echo "---- Get release note end. ----" >> /tmp/webs_upgrade.log
nvram set cfg_note=1
