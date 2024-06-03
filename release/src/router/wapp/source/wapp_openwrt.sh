#!/bin/sh

########################################
# define parameters
########################################

if [ -f /sbin/l1dat ]; then
	# variable interface name
	card1_main=`l1dat zone2if dev1 | awk '{print $1}'`
	card1_ext=`l1dat zone2if dev1 | awk '{print $2}'`
	card2_main=`l1dat zone2if dev2 | awk '{print $1}'`
	card2_ext=`l1dat zone2if dev2 | awk '{print $2}'`
	card3_main=`l1dat zone2if dev3 | awk '{print $1}'`
	card3_ext=`l1dat zone2if dev3 | awk '{print $2}'`

	card1_nvram="dev1"
	card2_nvram="dev2"
	card3_nvram="dev3"
else
	# fix interface name
	card1_main="ra0"
	card1_ext="ra"
	card2_main="rai0"
	card2_ext="rai"
	card3_main="rae0"
	card3_ext="rae"

	card1_nvram="2860"
	card2_nvram="rtdev"
	card3_nvram="wifi3"
fi

card1_exist=`ifconfig -a | grep ${card1_main}`
card2_exist=`ifconfig -a | grep ${card2_main}`
card3_exist=`ifconfig -a | grep ${card3_main}`

CONFDIR=/etc

########################################
# sanity check
########################################

#$1: 2860/rtdev , $2: ra0/rai0 $3: card_exist

killall wapp

### for certification use
if [ ! -f $CONFDIR/wapp_ap_ra0.conf ]; then
	cp $CONFDIR/wapp_ap_ra0_default.conf $CONFDIR/wapp_ap_ra0.conf
fi
if [ ! -f $CONFDIR/wapp_ap_rai0.conf ]; then
	cp $CONFDIR/wapp_ap_rai0_default.conf $CONFDIR/wapp_ap_rai0.conf
fi
if [ ! -f $CONFDIR/wapp_ap_wlan0.conf ]; then
	cp $CONFDIR/wapp_ap_wlan0_default.conf $CONFDIR/wapp_ap_wlan0.conf
fi
if [ ! -f $CONFDIR/wapp_ap_rax0.conf ]; then
	cp $CONFDIR/wapp_ap_rax0_default.conf $CONFDIR/wapp_ap_rax0.conf
fi

rm -rf /tmp/wapp*

wapp_intf_name=
########################################
# gen card1 config
########################################
all_inf_list="all_main_inf="
all_conf_list="conf_list="
if [ "$card1_exist" != "" ]; then
	card1_dat=`l1dat if2dat $card1_main | awk '{print $1}'`
	Bss_Num=`sed -n -e '/BssidNum/ s/.*\= *//p' $card1_dat`
	echo "$Bss_Num"
	all_inf_list="${all_inf_list}${card1_main};"    #add to all main inf list
	all_conf_list="${all_conf_list}$CONFDIR/wapp_ap_$card1_main.conf;"
	count=0
	while [ $count -lt $Bss_Num ]
	do
		wapp_intf_name="${wapp_intf_name} -c${card1_ext}${count}"
		count=$( expr $count + 1 )
	done
	echo "card 1 $wapp_intf_name"
fi

########################################
# gen card2 config
########################################

if [ "$card2_exist" != "" ]; then
	card2_dat=`l1dat if2dat $card2_main | awk '{print $1}'`
	Bss_Num=`sed -n -e '/BssidNum/ s/.*\= *//p' $card2_dat`
	echo "$Bss_Num"
	all_inf_list="${all_inf_list}${card2_main};"    #append to all main inf list
	all_conf_list="${all_conf_list}$CONFDIR/wapp_ap_$card2_main.conf;"
	count=0
	while [ $count -lt $Bss_Num ]
	do
		wapp_intf_name="${wapp_intf_name} -c${card2_ext}${count}"
		count=$( expr $count + 1 )
	done
	echo "card 2 $wapp_intf_name"
fi

########################################
# gen card3 config
########################################

if [ "$card3_exist" != "" ]; then
	card3_dat=`l1dat if2dat $card3_main | awk '{print $1}'`
	Bss_Num=`sed -n -e '/BssidNum/ s/.*\= *//p' $card3_dat`
	echo "$Bss_Num"
	all_inf_list="${all_inf_list}${card3_main};"    #append to all main inf list
	all_conf_list="${all_conf_list}$CONFDIR/wapp_ap_$card3_main.conf;"
	count=0
	while [ $count -lt $Bss_Num ]
	do
		wapp_intf_name="${wapp_intf_name} -c${card3_ext}${count}"
		count=$( expr $count + 1 )
	done
	echo "card 3 $wapp_intf_name"
fi

########################################
# write conf_list, inf_list & start wapp daemon
########################################
if [ "$all_conf_list" != "" ]; then
	echo "${all_conf_list}" > $CONFDIR/wapp_ap.conf
fi
if [ "$all_inf_list" != "" ]; then
	echo "${all_inf_list}" > $CONFDIR/wapp_main_inf.conf
	#wapp -d1 -v2
	#Fix 11R Roaming Fail Issue.
	wapp -d1 -v2 $wapp_intf_name
fi

