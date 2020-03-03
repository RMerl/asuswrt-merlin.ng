#!/bin/sh

wget_timeout=`nvram get apps_wget_timeout`
wget_options="-q -t 2 -T $wget_timeout"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

nvram set cfg_note=0
get_preferred_lang=`nvram get preferred_lang`
LANG="$get_preferred_lang"

# get release note
forsq=`nvram get apps_sq`

if [ $# != 2 ]; then
	echo "---- no model name or version ----" >> /tmp/webs_note.log
	nvram set cfg_note=2
	exit 0
fi

new_firm=`echo $2 | sed s/'\.'/_/4 | sed s/'\.'//g;`
echo "---- $new_firm ----" >> /tmp/webs_note.log

releasenote_file=$1_"$new_firm"_"$LANG"_note.zip
releasenote_file_US=$1_"$new_firm"_US_note.zip
releasenote_path="/tmp/release_note.txt"

wget_release=""
if [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	echo "---- download SQ beta_user release note ${dl_path_SQ_beta}${forsq}/$releasenote_file ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ_beta}${forsq}/$releasenote_file -O $releasenote_path
	if [ "$?" != "0" ]; then
		echo "---- download SQ beta_user release note ${dl_path_SQ_beta}${forsq}/$releasenote_file_US ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ_beta}${forsq}/$releasenote_file_US -O $releasenote_path
		wget_release=$?
	fi
elif [ "$forsq" == "1" ]; then
	echo "---- download SQ release note ${dl_path_SQ}/$releasenote_file ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ}/$releasenote_file -O $releasenote_path
	if [ "$?" != "0" ]; then
		wget $wget_options ${dl_path_SQ}/$releasenote_file_US -O $releasenote_path
	fi
	echo "---- download SQ release note ${dl_path_SQ}/$releasenote_file ----" >> /tmp/webs_note.log
else
	echo "---- download real release note for $1 ----" >> /tmp/webs_note.log
	wget $wget_options ${dl_path_file}/$releasenote_file -O $releasenote_path
	if [ "$?" != "0" ]; then
		wget $wget_options ${dl_path_file}/$releasenote_file_US -O $releasenote_path
	fi
	echo "---- download real release note ${dl_path_file}/$releasenote_file ----" >> /tmp/webs_note.log
fi

if [ "$?" != "0" ] || [ ! -s $releasenote_path ]; then
	echo "---- download release note failed ----" >> /tmp/webs_note.log
	nvram set cfg_note=3
	exit 0
fi
nvram set cfg_note=1
