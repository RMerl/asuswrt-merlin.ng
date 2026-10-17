#!/bin/sh
# hot plug script to add WDS, or semi dynamic wl vif to linux bridge
#
# wifi_setup.sh installs this scripts as default hotplug script
# if this is not the case, please add following lines in default hotplug script
# to execute this script on hotplug events.
# Also see cmwifi.mk.
#
# wifi_hotplug=/tmp/hotplug_wifi
# if [ -f $wifi_hotplug ]; then
#	$wifi_hotplug $1
# fi
#

export PATH=/usr/local/sbin:/usr/local/bin:/usr/local/usr/sbin:/usr/local/usr/bin:/usr/sbin:/usr/bin:/sbin:/bin
export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib/public:/usr/local/lib/private:/usr/local/lib/gpl:/lib:/usr/local/usr/lib

BCACPE_WIFI_INDICATION_FILE="/tmp/.brcm_bcacpe_wifi"
if [[ -e $BCACPE_WIFI_INDICATION_FILE ]]; then
	BCACPE_RDKB_BUILD=1
else
	BCACPE_RDKB_BUILD=0
fi

# network hotplug
bcm_set_dwds_br() {
	cmd=$1
	wds_if=$2

	wifi_br="UNKNOWN"

	parent_if=$(echo $wds_if | sed -e s/\.[^.]*$// -e s/wds/wl/ -e s/.0$//g)

	# find correct bridge based on current bridge reports
	tmp_file=$(mktemp)
	brctl show | grep -v interfaces > $tmp_file

	# wdsX.Y[.Z] vif parsing, to find its parent if name
	wifi_br=$( awk 'BEGIN{local_br="UNKNOWN";}
		{ if (NF==4) local_br=$1,$1=$4;
			if ($1 == "'$parent_if'") print local_br}' $tmp_file)

	rm $tmp_file

	if [ "$wifi_br" != "" ]; then
		brctl $cmd $wifi_br $wds_if
		if [ $?  -eq "0" ]; then
			ifconfig $wds_if up
		fi
		if [[ $BCACPE_RDKB_BUILD -ne 1 ]]; then
			# emf add/del interface
			emf_en=`nvram get emf_enable`
			if [ "$emf_en" == "1" ]; then
				[ $cmd == "addif" ] && emf add iface $wifi_br $wds_if
				[ $cmd == "delif" ] && emf del iface $wifi_br $wds_if
			fi
		fi
	fi
}

bcm_set_nvram_cfg_br() {
	cmd=$1
	vif=$2

	wifi_br=""

	# bridge lookup based on nvram lanX_ifname and lanX_ifnames
	for idx in 0 1 2 3 4
	do
		if [ "$idx" == "0" ]; then
			ifname="lan_ifname"
			ifnames="lan_ifnames"
		else
			ifname="lan${idx}_ifname"
			ifnames="lan${idx}_ifnames"
		fi
		lan_ifname=`nvram get $ifname`
		if [ "$lan_ifname" == "" ]; then
			continue
		fi
		lan_ifnames=`nvram get $ifnames`
		if [ "$lan_ifnames" == "" ]; then
			continue
		fi
		echo $lan_ifnames | grep $vif
		if [ $? -eq "0" ]; then
			wifi_br="$lan_ifname"
			break
		fi
	done

	if [ "$wifi_br" != "" ]; then
		# a check to workaround stalling bridge issue
		# of wlmngr2 mbss vif down up related missing mac evt
		for i in `seq 1 5`;
		do
			ifconfig $vif | grep UP
			if [ $?  -eq "0" ]; then
				break;
			fi
			sleep 1
		done

		brctl $cmd $wifi_br $vif
		if [ $?  -eq "0" ]; then
			ifconfig $vif up
		fi
	fi
}

# customized to add if to proper bridge
bcm_brctl() {
	cmd=$1
	vif=$2

	wds_prefix="wds"
	if [[ $vif =~ $wds_prefix ]]; then
		bcm_set_dwds_br $cmd $vif
	else
		bcm_set_nvram_cfg_br $cmd $vif
	fi

	if [ -e /proc/driver/macpt/mac ] ; then
		echo update > /proc/driver/macpt/mac
	fi
}

if [ "$1" = "net" -a -e /sys/class/net/$INTERFACE ]; then
	if [ "${INTERFACE//[.0-9]/}" = "wds" ] || [ "${INTERFACE//[.0-9]/}" = "wl" ]; then
		if [ "$ACTION" = "add" ]; then
			bcm_brctl addif $INTERFACE
			echo 1 > /sys/class/net/${INTERFACE}/netdev_group
			if [[ $BCACPE_RDKB_BUILD -ne 1 ]]; then
				echo "inf add $INTERFACE" > /proc/driver/flowmgr/cmd
			fi
		elif [ "$ACTION" = "remove" ]; then
			ifconfig $INTERFACE down
			bcm_brctl delif $INTERFACE
			if [[ $BCACPE_RDKB_BUILD -ne 1 ]]; then
				echo "inf del $INTERFACE" > /proc/driver/flowmgr/cmd
			fi
		fi
	fi
fi
