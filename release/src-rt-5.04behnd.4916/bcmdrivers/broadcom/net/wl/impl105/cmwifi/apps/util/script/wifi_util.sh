#!/bin/sh

if [ "`which nvram`" == "" ]; then
	# Make sure nvram/libs are in paths.
	PATH=$PATH:/usr/local/sbin:/usr/bin
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/lib
fi

# DSL/PON have wl_mlo_config as kernel nvram, hence using kget
# to retrieve the value, CM has aliase kget the same as get.
g_mlo_cfg=`nvram kget wl_mlo_config`

g_mlo_enabled=0
[[ -n $g_mlo_cfg && $g_mlo_cfg =~ .*0.* ]] && g_mlo_enabled=1
# for DSL/PON platform edpd feature,to indicate if dpd is in dpd_in_progress
# when rc restart or restart restart_security_daemons.
g_dpd_in_progress=0
[[ $BCACPE_RDKB_BUILD -ne 1 && -f /tmp/dpd_in_progress ]] && g_dpd_in_progress=1

# Return "1" if onewifi is enabled; Return "0" otherwise.
is_one_wifi() {
	if [ -e $ONE_WIFI_BUILD_FILE ]; then
		echo "1"
	else
		echo "0"
	fi
}

# Return "1" if platform is RDK; Return "0" otherwise.
is_rdk() {
	if [ -e "/etc/utopia/system_defaults"  ]; then
		echo "1"
	else
		echo "0"
	fi
}

# Return 1 if nas/wps_monitor is disabled; Return 0 when hostapd is used
is_hostapd_disabled() {
	local hapd_enabled=`nvram get hapd_enable`
	if [ "$hapd_enabled" == "0" ] || [ "$hapd_enabled" == "" ]; then
		echo 1
	else
		echo 0
	fi
}

# Return 1 if hspot is enabled, otherwise return 0
# Translated from wlmngr.c::enableHspot()
# TODO: To make the script simple, script just launch the app, app itself
# check and exit if it is not supposed to be running.
is_hspot_enabled() {
	for osifname in $@
	do
		nvifname=$(osifname_to_nvifname $osifname)
		if [ "`nvram get ${nvifname}_bss_enabled`" = "1" ] && [ "`nvram get ${nvifname}_hspot`" = "1" ]; then
			echo 1
			return
		fi
		vifs=`nvram get ${nvifname}_vifs`
		for vifname in $vifs
		do
			if [ "`nvram get ${vifname}_bss_enabled`" = "1" ] && [ "`nvram get ${vifname}_hspot`" = "1" ]; then
				echo 1
				return
			fi
		done
	done
	echo 0
}

# Translated from shutils.c::osifname_to_nvifname()
# Not support bridge, wds, virtual interface
osifname_to_nvifname() {
	if [ -n "`echo $1|sed -n '/^wl/p'`" ]; then
		echo $1
		return
	fi
	local radio_idx=0
	local num_of_radios=3
	if [ -e /tmp/wl_instances.txt ]; then
		num_of_radios=`cat /tmp/wl_instances.txt`
	fi
	while [ $radio_idx -lt $num_of_radios ]
	do
		if [ $1 = `nvram get wl${radio_idx}_ifname` ]; then
			echo wl${radio_idx}
			return
		fi
		radio_idx=`expr $radio_idx + 1`
	done
	echo ""
}

# Translated from shutils.c::nvifname_to_osifname()
nvifname_to_osifname() {
	if [ -n "`echo $1|sed -n '/\./p'`" ] || [ -z "`echo $1|sed -n '/^wl/p'`" ]; then
		echo $1
	else
		echo `nvram get $1_ifname`
	fi
}

# Return bsscfg_idx for (primary or virtual) wl interface; Return "" for not a wl IF.
wlif_idx() {
	idx=`wl -i $1 bsscfg_idx 2>/dev/null`
	echo "$idx"
}

# Return "1" if arg is primary wl interfaces; Return "0" otherwise.
is_primary_wlif() {
	if [ "$(wlif_idx $1)" == "0" ]; then
		echo "1"
	else
		echo "0"
	fi
}

# Return parimary interface name from input list; Return "" if there is no primary IF
get_primary_ifnames() {
	ifnames=""
	for name in $*
	do
		if [ "$(is_primary_wlif $name)" == "1" ]; then
			if [ "$ifnames" == "" ]; then
				ifnames="$name"
			else
				ifnames="$ifnames $name"
			fi
		fi
	done

	echo "$ifnames"
}

# Return primary interface name based on interface name or radio index; Return lan_ifnames when user input is missing
get_wifi_ifnames() {
	lan_ifnames=`nvram get lan_ifnames`

	if [ -z "$*" ]; then
		# No IFs are specified, use the nvram values
		wifi_ifnames=$(get_primary_ifnames $lan_ifnames)
	elif [ -z "`echo $* | sed -n '/^[0-9]/p'`" ]; then
		# ifname is speficied
		wifi_ifnames=$(get_primary_ifnames $*)
	else
		# radio index is specified, convert to ifname
		wifi_ifnames=""
		for i in $*
		do
			wifi_ifnames="${wifi_ifnames} `nvram get wl${i}_ifname`"
		done
	fi

	echo $wifi_ifnames
}

get_wl_instances() {
	i=0

	for name in $*
	do
		if [ "$(wlif_idx $name)" != "" ]; then
			((i++))
		fi
	done

	echo "$i"
}

# Get the pid of the given app.
get_app_pid() {
	if [ "$(is_rdk)" == "0" ]; then
		echo `ps -C $1 -o pid=`
	else
		echo `pidof $1`
	fi
}

# The restart_app() starts the given app if it is not running,
# or sends a restart signal if it is currently running.
# Use single quote to pass arguments, for example:
#    restart_app 'telnetd -b 192.168.0.1 -l /bin/sh &'
restart_app() {
	if [ "$(get_app_pid $1)" == "" ];  then
		print_to_wifi_log "<$FUNCNAME> Starting $@";
		sh -c "$@"
	else
		print_to_wifi_log "<$FUNCNAME> Restarting $@";
		# Send signals
	fi
}

# Get the bridge name.
function check_eth {
	set -o pipefail
	ethtool "$1" | grep -q "Link detected"
}

function get_bridge_name {
	if [ "$(is_rdk)" == "1" ]; then
		echo "brlan0"
	elif check_eth wanbridge; then
		echo "wanbridge"
	else
		echo "br0"
	fi
}

# Delete all virtual interfaces of the given primary IF list. Primary IFs cannot be deleted.
function del_wlifs() {
	if [ $(is_hostapd_disabled) -eq 0 ]; then
		iw_cmd=1
	else
		iw_cmd=0
	fi
	print_to_wifi_log "<$FUNCNAME> on $@"
	for osifname in $@
	do
		nvifname=$(osifname_to_nvifname $osifname)
		for i in `seq 1 $MAX_NUM_VIFS`
		do
			vifname=`nvram get $nvifname.${i}_ifname`
			if [ "$vifname" != "" ]; then
				echo "Deleting $vifname ..."
				if [ "$iw_cmd" == "1" ]; then
					iw dev $vifname del 2>/dev/null
				else
					wl -i $osifname interface_remove -C $i
				fi
			fi
		done
	done
}

# Remove the wl interfaces from the lanX_ifname bridge if it is no longer in the lanX_ifnames.
# It will cover cases when lanX_ifname and lanX_ifnames nvram are changed.
del_wlifs_from_brifs() {
	local _radio_index=-1
	if [[ $# -gt 0 ]]; then
		_radio_index=$(get_radio_index_by_name $1)
	fi
	print_to_wifi_log "<$FUNCNAME> $#:$@, _radio_index:$_radio_index"

	virtual_nets=`ls /sys/devices/virtual/net`
	for net in $virtual_nets
	do
		if [ ! -e "/sys/devices/virtual/net/$net/brif" ]; then
			# Not a bridge
			continue
		fi
		idx_max=`expr $MAX_NUM_BRIDGE - 1`
		for idx in `seq 0 $idx_max`
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
				continue;
			fi
			if [ "$lan_ifname" == "$net" ]; then
				break 1
			fi
		done
		if [ "$net" != "$lan_ifname" ]; then
			# The bridge is not configured in any lanX_ifname
			# Delete all wl interfaces under this bridge.
			del_all_wlifs=1
		else
			# The bridge is configured in lanX_ifname.
			# Delete the wl interfaces that are not in lanX_ifnames
			del_all_wlifs=0
		fi

		br_ifs=`ls /sys/devices/virtual/net/$net/brif`
		lan_ifnames=`nvram get $ifnames`
		for name in $br_ifs
		do
			if [ $del_all_wlifs == 0 ] && [ "${lan_ifnames/$name/}" != "$lan_ifnames" ]; then
				# Already in this lanX_ifname list
				continue
			fi
			if [ "$(wlif_idx $name)" == "" ]; then
				# Not a wl interface
				continue
			fi
			ifconfig $name down
			# when restarting single radio, should not remove inteface of the other radios. especially for wds interface
			if [[ $_radio_index -lt 0 ]] || [[ $(get_radio_index_by_name $name) -eq $_radio_index ]]; then
				print_to_wifi_log "<$FUNCNAME> brctl delif $net $name"
				brctl delif $net $name 2>/dev/null
			fi
			if [[ $BCACPE_RDKB_BUILD -ne 1 ]]; then
				echo "inf del $name" > /proc/driver/flowmgr/cmd
			fi
		done
	done
}

flowmgr_wait () {

    if [[ $BCACPE_RDKB_BUILD -eq 1 ]]; then
        echo "wlssk platform does not need flowmgr"
        return;
    fi
    x=0;
    y=0;
    # Wait until flowmgr configured, timeout after 20sec
    until [ $x -eq 1 ] || [ $y -ge 20 ]
    do
        lsmod|grep flowmgr > /dev/null
        if [ $? -eq 0 ]; then
            x=1;
        fi
        sleep 1
        y=$(($y+1))
    done
}

# Add the wl interfaces to the lanX_ifname bridge if it is in the lanX_ifnames.
# Param $1="primary_wlifs" is required by acsd2: brctl addif bridge wlX and ifconfig wlX up only
# Param $2=do "brctl add" if "$2" == "1"
# Param $3=do flowmgr config if "$3" == "1"
add_wlifs_to_brifs() {
	idx_max=`expr $MAX_NUM_BRIDGE - 1`
	for idx in `seq 0 $idx_max`
	do
		if [ "$idx" == "0" ]; then
			ifname="lan_ifname"
			ifnames="lan_ifnames"
			ifhwaddr="lan_hwaddr"
		else
			ifname="lan${idx}_ifname"
			ifnames="lan${idx}_ifnames"
			ifhwaddr="lan${idx}_hwaddr"
		fi
		lan_ifname=`nvram get $ifname`
		if [ "$lan_ifname" == "" ]; then
			continue;
		fi
		lan_ifnames=`nvram get $ifnames`
		for name in $lan_ifnames
		do
			if [ "$(wlif_idx $name)" == "" ]; then
				# Not a wl interface
				continue
			fi
			if [ "$2" == "1" ]; then
				brctl addif $lan_ifname $name 2>/dev/null
			fi

			ifconfig $name up
			if [ "$1" == "primary_wlifs" ]; then
				# Required by acsd2, ONLY add wlX to main bridge and ifconfig wlX up
				continue
			fi

			if [ "$3" == "1" ]; then
				if [[ $BCACPE_RDKB_BUILD -ne 1 ]]; then
					flowmgr_wait
					echo "inf add $name" > /proc/driver/flowmgr/cmd
				fi
			fi
		done
		if [ "$1" == "primary_wlifs" ]; then
			# Required by acsd2, ONLY add wlX to main bridge and ifconfig wlX up
			return
		fi
		hwaddr=`ifconfig ${lan_ifname} | awk '/HWaddr/ {print $5}'`
		nvram set $ifhwaddr=$hwaddr

		if [ ! -e $BCACPE_WIFI_INDICATION_FILE ]; then
			dor=`dhd -i wl${idx} rnr_rxoffload`
			if [[ $? -eq 0 ]]; then
				dor=`echo ${dor} | sed 's/\\r//g'`
				if [[ "${dor}" == "1" ]]; then
					echo 1 > /proc/sys/net/flowmgr/use_tcp_3way_handshake
				fi
			fi
		fi
	done
}

save_boot_time() {
	upSecs=$(cat /proc/uptime | awk -F '.' '{print $1}')
	nvname="$1_boot_time"
	print_to_wifi_log "<$FUNCNAME> nvram set $nvname=$upSecs"
	nvram set $nvname=$upSecs
}

# Stop(Down) the given primary interfaces in the args.
do_wl_down() {
	print_to_wifi_log "<$FUNCNAME> on $*"
	for name in $*
	do
		if [ "$(wlif_idx $name)" != "" ]; then
			wlconf $name down > /dev/null
		fi
	done
}

# Do wlconf on the given primary interfaces in the args.
do_wl_up() {
	print_to_wifi_log "<$FUNCNAME> on $*"
	for name in $*
	do
		if [ "$(wlif_idx $name)" != "" ]; then
			wlconf $name up > /dev/null
			save_boot_time $name
		fi
	done
}

# Config the given primary interfaces in the args. For example:
# do_wl_conf wl0 wl1
do_wl_config() {
	for name in $*
	do
		if [ "$(wlif_idx $name)" == "" ]; then
			continue
		fi
		wlconf $name up > /dev/null
		# wl=$(get_wl_nv_prefix $name)		# Convert ethX to wlX
		wl=$name
		wl -i $name down
		if [ "`nvram get ${wl}_mode`" == "psr" ]; then
			psr_mac=`nvram get ${wl}_hwaddr`
			ifconfig ${wl}.1 hw ether $psr_mac
		else
			for vifname in `nvram get ${wl}_vifs`
			do
				if [ "`nvram get ${vifname}_bss_enabled`" == "1" ]; then
					vifhwaddr=`nvram get ${vifname}_hwaddr`
					ifconfig $vifname hw ether $vifhwaddr
				fi
			done
		fi
		wl -i $name up
	done
}

# Call before stopping hostapd to ensure wl interfaces and vifs down
do_wl_forcedown() {
	# down all interfaces for mlo
	if [[ $g_mlo_enabled -eq 1 ]]; then
		for name in $WIFI_IFNAMES
		do
			do_wl_down $name
			do_ifconfig_wlifs_down $name
		done
	fi
}

wlconf_mlo_forceup() {
	# wlconf forceup for mlo
	if [[ $g_mlo_enabled -eq 1 ]]; then
		for name in $WIFI_IFNAMES
		do
			echo "forceup $name"
			wlconf $name forceup
		done
	fi
}

# Call before starting hostapd to ensure wl up for mlo
do_wl_forceup() {
	# wlconf forceup for mlo
	if [[ $g_mlo_enabled -eq 1 ]]; then
		for name in $WIFI_IFNAMES
		do
			local nvifname=$(osifname_to_nvifname $name)
			local radio_enabled=`nvram get ${nvifname}_radio`
			local radio_idx=`echo $nvifname|sed -n 's/^wl\([0-9]\+\)$/\1/p'`
			if [[ $BCACPE_RDKB_BUILD -eq 1 ]]; then
				local wldpd=$(nvram kget ${nvifname}_dpd)
				if [[ -n $wldpd && $wldpd -eq 1 ]]; then
					print_to_wifi_log "<$FUNCNAME>, radio $radio_idx is in powerdown mode, no need to bringup interface"
					continue
				fi
			fi
			if [ -z "$radio_enabled" ] || [ "$radio_enabled" = "0" ]; then
				print_to_wifi_log "<$FUNCNAME>, radio $radio_idx disabled, no need to bringup interface"
				continue
			fi
			do_wl_up $name
			do_ifconfig_wlifs_up $name
		done
		wlconf_mlo_forceup
	fi
}

# Disable IPv6 on all primary and virtual interfaces
# do_disable_ipv6 wl0 wl1 wl2
do_disable_ipv6() {
	for name in $*
	do
		echo 1 > /proc/sys/net/ipv6/conf/${name}/disable_ipv6 2> /dev/null
		for vifname in `nvram get ${name}_vifs`
		do
			echo 1 > /proc/sys/net/ipv6/conf/${vifname}/disable_ipv6 2> /dev/null
		done
	done
}

# Start the given primary interfaces in the args.
do_wl_start() {
	for name in $*
	do
		wlconf $name start > /dev/null
	done
}

# Kill application.
# Remove its entry under /tmp/dm to unregister from debug monitor firstly
do_app_stop() {
	print_to_wifi_log "<$FUNCNAME> $1"
	pid=`pidof $1`
	if [ -n "$pid" ]; then
		rm /tmp/dm/$pid
		if [ $? -ne 0 ]; then
			print_to_wifi_log "<$FUNCNAME> fail to unregister $1 from debug monitor"
		fi
		kill $2 $pid
		if [ $? -ne 0 ]; then
			print_to_wifi_log "<$FUNCNAME> fail to kill $1"
		fi
	else
		print_to_wifi_log "<$FUNCNAME> $1 not launched, no need to kill!"
	fi
}

# Stop the security daemons on given radio in the args.
stop_security_daemons() {
	print_to_wifi_log "<$FUNCNAME> on $@"
	local hapd_disabled=$(is_hostapd_disabled)
	if [ $hapd_disabled -eq 0 ]; then

		if [[ $g_mlo_enabled -eq 1 ]]; then
			do_wl_forcedown

			# if dpd is ongoing and individual radio is requested to stop hostapd,ignore
			# it as dpd won't need hostapd restart in mlo mode.
			if [[ $# -eq 1 && $g_dpd_in_progress -eq 1 ]]; then
				print_to_wifi_log " mlo&dpd mode, no hapd_conf individual interface stop"
			else
				# Stop the single hostapd instance.
				print_to_wifi_log "<$FUNCNAME>, stop the single hostapd instance"
				hapd_conf stop
			fi
			return 0
		else
			for osifname in $@
			do
				do_wl_down $osifname
				do_ifconfig_wlifs_down $osifname

				local nvifname=$(osifname_to_nvifname $osifname)
				local radio_idx=`echo $nvifname|sed -n 's/^wl\([0-9]\+\)$/\1/p'`

				print_to_wifi_log "<$FUNCNAME>, stop hostpad on radio $radio_idx"
				hapd_conf $radio_idx stop
			done
		fi
	else
		do_app_stop wps_monitor -15
		sleep 1
		do_app_stop nas -9
	fi
	do_app_stop eapd -9
	do_app_stop bsd -15
	print_to_wifi_log "<$FUNCNAME> on $@ done"
}

# Start the security daemons on given radio in the args.
start_security_daemons() {
	print_to_wifi_log "<$FUNCNAME> on $@"
	local hapd_disabled=$(is_hostapd_disabled)

	# start eapd before hostpad to avoid event missing
	restart_app eapd

	if [ $hapd_disabled -eq 0 ]; then

		if [[ $g_mlo_enabled -eq 1 ]]; then
			do_wl_forceup

			# MAP is found in mlo_cfg. Start single hostapd instance for all radios.
			print_to_wifi_log "<$FUNCNAME>, start single hostapd instance for all radios"
			# it as dpd won't need hostapd restart in mlo mode.
			if [[ $# -eq 1 && $g_dpd_in_progress -eq 1 ]]; then
				print_to_wifi_log " mlo&dpd mode, no hapd_conf individual interface start"
			else
				if [ "$RDK" = "1" ]; then
					hapd_conf start >> $WLAN_APPS_LOG_FILE
					ps | grep hostapd >> $WLAN_APPS_LOG_FILE
				else
					hapd_conf start
				fi
			fi
		else
			for osifname in $@
			do
				local nvifname=$(osifname_to_nvifname $osifname)
				local radio_enabled=`nvram get ${nvifname}_radio`
				local radio_idx=`echo $nvifname|sed -n 's/^wl\([0-9]\+\)$/\1/p'`
				if [[ $BCACPE_RDKB_BUILD -eq 1 ]]; then
					local wldpd=$(nvram kget ${nvifname}_dpd)
					if [[ -n $wldpd && $wldpd -eq 1 ]]; then
						print_to_wifi_log "<$FUNCNAME>, radio $radio_idx is in powerdown mode, no need to start security daemons"
						continue
					fi
				fi
				if [ -z "$radio_enabled" ] || [ "$radio_enabled" = "0" ]; then
					print_to_wifi_log "<$FUNCNAME>, radio $radio_idx disabled, no need to start security daemons"
					continue
				fi
				do_wl_up $osifname
				do_ifconfig_wlifs_up $osifname

				print_to_wifi_log "<$FUNCNAME>, start hostpad on radio $radio_idx"
				if [ "$RDK" = "1" ]; then
					hapd_conf $radio_idx start >> $WLAN_APPS_LOG_FILE
				else
					hapd_conf $radio_idx start
				fi

				# wait for all BSSes to be up
				sleep 1

				do_wl_start $osifname

			done
		fi

		#reload wps_pbcd
		killall -SIGUSR1 wps_pbcd
	else
		restart_app nas
		rm -rf /tmp/wps_monitor.pid
		restart_app wps_monitor
	fi
	restart_app bsd
	return 0
}

# Start the interfaces on given radio in the args.
launch_wifi_apps() {
        print_to_wifi_log "<$FUNCNAME>"

        #reload wps_pbcd
        killall -SIGUSR1 wps_pbcd
        restart_app eapd
        restart_app bsd
        restart_app ceventd
        return 0
}

restart_security_daemons() {
	print_to_wifi_log "<$FUNCNAME> on $@"
	if [ "$@" = "all" ] || [ "$@" = "" ]; then
		hapd_conf stop
		hapd_conf start
	else
		stop_security_daemons $@
		start_security_daemons $@
	fi
	print_to_wifi_log "<$FUNCNAME> on $@ done"
}

# Launch permanent apps that don't require restart by referring wlmngr2
launch_permanent_apps() {
	print_to_wifi_log "<$FUNCNAME> $@"

	if [ "$(is_one_wifi)" == "0" ]; then
		# acsd2 for RDK, launch once during boot
		if [ "$RDK" = "1" ]; then
			start_acsd2 1 &
		fi
	fi

	local hapd_disabled=$(is_hostapd_disabled)

	if [ "$RDK" = "1" ]; then
		nvram set ecbd_enable=1
		print_to_wifi_log "<$FUNCNAME> launch ecbd"
		ecbd 2> /dev/null &

		if [ $hapd_disabled -eq 0 ]; then
			print_to_wifi_log "<$FUNCNAME> launch wps_pbcd"
			wps_pbcd >> $WLAN_APPS_LOG_FILE
		fi
	else
		if [ $hapd_disabled -eq 0 ]; then
			print_to_wifi_log "<$FUNCNAME> launch wps_pbcd"
			wps_pbcd
		fi
	fi

	# debug_monitor
	local enable="`nvram get debug_monitor_enable`"
	if [ -z "$enable" ] || [ "$enable" = "0" ]; then
		print_to_wifi_log "<$FUNCNAME> debug_monitor not enabled"
	else
		if [ "$(get_app_pid 'debug_monitor')" == "" ]; then
			rm -rf /tmp/dm
			mkdir /tmp/dm

			local crash_log_dir="`nvram get crash_log_backup_dir`"
			if [ -z "$crash_log_dir" ]; then
				if [ "$RDK" = "1" ]; then
					# this is XB8 default; also for RDKM
					crash_log_dir="/nvram2/wifiCrashLogs"
				else
					# Generic
					crash_log_dir="/mnt/flash/wifiCrashLogs"
				fi
			fi
			print_to_wifi_log "<$FUNCNAME> launch debug_monitor $crash_log_dir"
			# Note: crash_log_dir needs to be RW
			mkdir -p "$crash_log_dir"
			debug_monitor "$crash_log_dir"
		else
			print_to_wifi_log "<$FUNCNAME> debug_monitor is already running"
		fi
	fi
}

stop_permanent_apps() {
	print_to_wifi_log "<$FUNCNAME> $@"

	# print_to_wifi_log "<$FUNCNAME> stop ecbd"
	killall -q -9 ecbd

	local hapd_disabled=$(is_hostapd_disabled)
	if [ $hapd_disabled -eq 0 ]; then
		print_to_wifi_log "<$FUNCNAME> stop wps_pbcd"
		killall -q -9 wps_pbcd
	fi

	print_to_wifi_log "<$FUNCNAME> stop debug_monitor"
	killall -q -9 debug_monitor
	rm -r /tmp/dm

	if [ "$(is_one_wifi)" == "0" ]; then
		print_to_wifi_log "<$FUNCNAME> stop acsd2"
		killall -q -9 acsd2
	fi
}

get_wl_mlo_map_if()
{
	local is_map="-1";
	local map_if="-1";
	# find mlo map and set map_if
	is_map=`echo ${g_mlo_cfg} | cut -d " " -f 1`
	[[ "$is_map" -eq "0" ]] && map_if="wl0"

	is_map=`echo ${g_mlo_cfg} | cut -d " " -f 2`
	[[ "$is_map" -eq "0" ]] && map_if="wl1"

	is_map=`echo ${g_mlo_cfg} | cut -d " " -f 3`
	[[ "$is_map" -eq "0" ]] && map_if="wl2"

	is_map=`echo ${g_mlo_cfg} | cut -d " " -f 4`
	[[ "$is_map" -eq "0" ]] && map_if="wl3"

	#return map ap ifname
	echo "$map_if"
}

#Stop(Down) mlo operational mode
do_wl_mlo_down()
{
	map_if=$(get_wl_mlo_map_if)

	if [[ ! "$map_if" -eq "-1" ]] ; then
		print_to_wifi_log "<$FUNCNAME> on $map_if"
		wl -i $map_if mlo_down 2>/dev/null
	fi
}

#Enable(Up) mlo operational mode
do_wl_mlo_up()
{
	map_if=$(get_wl_mlo_map_if)
	if [[ ! "$map_if" -eq "-1" ]] ; then
		print_to_wifi_log "<$FUNCNAME> on $map_if"
		wl -i $map_if mlo_up 2>/dev/null
	fi

}
# Stop all applications. If force is given as argument it kills the app regardless
do_apps_stop() {
	print_to_wifi_log "<$FUNCNAME> $@"

	local force=0
	if [ $# -gt 1 ] && [ "$1" = "force" ]; then
		force=1
		shift
	fi

	stop_security_daemons $@

	if [ $force -eq 1 ] && [ "${PHASE2_SEPARATE_RC}" == "0" ]; then
		killall wlmngr2
	fi

	if [ "$RDK" = "0" ]; then
		do_app_stop acsd2 -9
	fi

	killall -q -9 ceventd
	killall -q -9 ssd

	# Appeventd
	if [ $force -eq 1 ] || [ "`nvram get appeventd_enable`" != "1" ]; then
		killall appeventd
	fi

	# Eventd
	if [ $force -eq 1 ] || [ "`nvram get eventd_enable`" != "1" ]; then
		killall eventd
	fi

	# toad
	do_app_stop toad -15

	# wbd
	do_app_stop wbd_master -9
	do_app_stop wbd_slave -9

	# hspotap
	do_app_stop hspotap -15

	# qosmgmtd
	do_app_stop qosmgmtd
}

# Restart all applications.
do_apps_restart() {
	# Apps shall check its own nvram enable variable, and then decides what to do.
	print_to_wifi_log "<$FUNCNAME> $@"

	# security daemons, e.g. eapd, hostapd, etc
	start_security_daemons $@

	if [ "$RDK" = "0" ]; then
		restart_app 'acsd2 &'
	else
		if [ "$(is_one_wifi)" == "0" ]; then
			# Start_acsd2 here if acsd2 is not launched in start_wifi()
			if [ "$(get_app_pid 'acsd2')" == "" ]; then
				start_acsd2 0 &
			fi
		fi
	fi

	# ceventd
	if [ "`nvram get ceventd_enable`" = "1" ]; then
		restart_app 'ceventd'
	fi

	# ssd
	if [ "`nvram get ssd_enable`" = "1" ]; then
		restart_app 'ssd &'
	fi

if [ "${PHASE2_SEPARATE_RC}" == "0" ]; then
	restart_app 'wlmngr2 &'
fi

	# Appeventd
	if [ "`nvram get appeventd_enable`" == "1" ]; then
		restart_app 'appeventd &'
	fi

	# Eventd
	if [ "`nvram get eventd_enable`" == "1" ]; then
		restart_app 'eventd &'
	fi

	# toad
	toad_ifnames="`nvram get toad_ifnames`"
	if [ -n "$toad_ifnames" ]; then
		restart_app "toad -I \"${toad_ifnames}\" &"
	fi

	# wbd
	restart_app wbd_master
	restart_app wbd_slave

	# hspotap
	if [ $(is_hspot_enabled $@) -eq 1 ]; then
		restart_app 'hspotap &'
	fi

	# qosmgmtd
	restart_app qosmgmtd
}

do_apps_start() {
	do_apps_restart $@
}

# Stop traffic from wifi to host
do_wlifs_down() {
	do_wl_mlo_down
	for osifname in $@
	do
		wl -i $osifname down 2>/dev/null
		ifconfig $osifname down 2>/dev/null
		nvifname=$(osifname_to_nvifname $osifname)
		for i in `seq 1 $MAX_NUM_VIFS`
		do
			vifname=`nvram get $nvifname.${i}_ifname`
			if [ "$vifname" != "" ]; then
				ifconfig $vifname down 2>/dev/null
			fi
		done
	done
	sleep 2
}

# ifconfig up
do_ifconfig_wlifs_up() {
	print_to_wifi_log "<$FUNCNAME> on $@"
	local nvifname=$(osifname_to_nvifname $1)
	ifconfig $1 up 2>/dev/null
	vifs=`nvram get ${nvifname}_vifs`
	for i in $vifs
	do
		ifconfig $i up 2>/dev/null
	done
}

# ifconfig down
do_ifconfig_wlifs_down() {
	print_to_wifi_log "<$FUNCNAME> on $@"
	ifconfig $1 down 2>/dev/null

	local nvifname=$(osifname_to_nvifname $1)
	vifs=`nvram get ${nvifname}_vifs`
	for i in $vifs
	do
		ifconfig $i down 2>/dev/null
	done
}

# load defaults from /tmp/factory_nvram.data, only for TCH Comcast
load_factory_nvram() {
	filename=/tmp/factory_nvram.data
	if [ ! -e "$filename" ]; then
		print_to_wifi_log "<$FUNCNAME> $filename does not exist"
		return
	fi

	# Default 5.0 GHz SSID has the same value with 2.4 GHz SSID
	if [ -z "`nvram get wl_ssid`" ]; then
		ssid=`grep "Default 2.4 GHz SSID" "$filename" | cut -d ":" -f 2- | cut -d " " -f 2-`
		print_to_wifi_log "<$FUNCNAME> default ssid: $ssid"
		nvram set wl_ssid="$ssid"
	fi

	if [ -z "`nvram get wl_wpa_psk`" ]; then
		psk=`grep "Default WIFI Password" "$filename" | cut -d ":" -f 2- | cut -d " " -f 2-`
		print_to_wifi_log "<$FUNCNAME> default psk: $psk"
		nvram set wl_wpa_psk="$psk"
	fi

	if [ -z "`nvram get wps_device_pin`" ]; then
		wps_pin=`grep "Default WPS Pin" "$filename" | cut -d ":" -f 2- | cut -d " " -f 2-`
		print_to_wifi_log "<$FUNCNAME> default wps pin: $wps_pin"
		nvram set wps_device_pin="$wps_pin"
	fi
	print_to_wifi_log "<$FUNCNAME> done."
}

# find name of the bridge to which the specified interface is added
find_bridge_name() {
	idx=0
	while [ $idx -lt $MAX_NUM_BRIDGE ]; do
		if [ $idx -eq 0 ]; then
			nv_name="lan_ifname"
		else
			nv_name="lan${idx}_ifname"
		fi
		lan_ifnames="`nvram get ${nv_name}s`"
		for name in $lan_ifnames
		do
			if [ "$name" = "$1" ]; then
				echo "`nvram get $nv_name`"
				return
			fi
		done
		id=$(( $idx + 1 ))
	done
	echo ""
}

# setup and start emf, only called once
emf_start() {
	if [ "`nvram get emf_enable`" != "1" ]; then
		print_to_wifi_log "<$FUNCNAME> emf is disabled"
		return
	fi

	for ifname in $@
	do
		bridge_name=$(find_bridge_name $ifname)

		if [ -z "$bridge_name" ]; then
			print_to_wifi_log "<$FUNCNAME> $ifname is not in any bridge"
			continue
		fi

		print_to_wifi_log "<$FUNCNAME> for ${bridge_name}:${ifname}"

		emf add bridge $bridge_name
		igs add bridge $bridge_name
		emf start $bridge_name
	done
}

# stop emf, only called once
emf_stop() {
	for ifname in $@
	do
		bridge_name=$(find_bridge_name $ifname)

		if [ -z "$bridge_name" ]; then
			print_to_wifi_log "<$FUNCNAME> $ifname is not in any bridge"
			continue
		fi

		print_to_wifi_log "<$FUNCNAME> for ${bridge_name}:${ifname}"

		emf stop $bridge_name
	done
}

# add interface to emf, called when restart radio(s) through 'wifi_setup.sh restart'
emf_add_iface() {
	if [ "`nvram get emf_enable`" != "1" ]; then
		print_to_wifi_log "<$FUNCNAME> emf is disabled"
		return
	fi

	for ifname in $@
	do
		bridge_name=$(find_bridge_name $ifname)

		if [ -z "$bridge_name" ]; then
			print_to_wifi_log "<$FUNCNAME> $ifname is not in any bridge"
			continue
		fi

		print_to_wifi_log "<$FUNCNAME> for ${bridge_name}:${ifname}"

		# TODO: ideally, here we only add specified WiFi interface.
		for brif in `ls /sys/devices/virtual/net/${bridge_name}/brif`
		do
			print_to_wifi_log "emf add iface $bridge_name $brif"
			emf add iface $bridge_name $brif
		done

		dhd -i ${ifname} wmf_bss_enable 1
		if [ $? -eq 0 ]; then # FD
			dhd -i ${ifname} wmf_ucast_igmp_query 1
		else # NIC
			wl -i ${ifname} wmf_bss_enable 1
			wl -i ${ifname} wmf_ucast_igmp_query 1
		fi
	done
}

# delete interface from emf, called when restart radio(s) through 'wifi_setup.sh restart'
emf_del_iface() {
	for ifname in $@
	do
		bridge_name=$(find_bridge_name $ifname)

		if [ -z "$bridge_name" ]; then
			print_to_wifi_log "<$FUNCNAME> $ifname is not in any bridge"
			continue
		fi

		print_to_wifi_log "<$FUNCNAME> for ${bridge_name}:${ifname}"

		emf del iface $bridge_name $ifname
	done
}

# direct and print message to application log file
print_to_wifi_log() {
        if [ "$RDK" == "0" ]; then
                echo "`date` [$0] [$PPID] $*"
        else
                echo "`date` [$0] [$PPID] $*" >>  $WLAN_APPS_LOG_FILE
        fi
}

# This is to debug pcie registers. Change baseaddr and numreg as needed
pcie_baseaddr()
{
	baseaddr=0
	if [ $# -eq 1 ]; then
		if [ "$1" == "wl0" ]; then
			baseaddr=0xf1080000
		elif [ "$1" == "wl1" ]; then
			baseaddr=0xf10c0000
		elif [ "$1" == "wl2" ]; then
			#TBD - gives error when no wl2
			#baseaddr=0xf1090000
			baseaddr=0
		fi
	fi
	echo $baseaddr
}

pcie_dumpregs_from_offset()
{
	if [ $# -ne 3 ]; then
		echo NumArg = $#
		echo "Usage: pcie_dumpregs_range <ifname> <offset> <numreg>"
		return
	else
		paddr=$(pcie_baseaddr $1)
		# get to 4byte boundary
		ibase=16; offset=$2
		num=$(( $offset % 4 ));
		offset=$(( $offset - $num ));
		# get to start address
		paddr=$(( $paddr + $offset ))
		numreg=$3
		regoffset=0
		lastoffset=`expr $numreg \* 4`
		echo "---$1 PCIE Host Registers-----------"
		while [ $regoffset -lt  $(($lastoffset)) ]
		do
			addr=$(( $paddr + $regoffset))
			addrx=`printf 0x%X $addr`
			val=`devmem $addrx`
			printf ' 0x%04x : 0x%08x \n' $((addrx)) $((val))
			regoffset=`expr $regoffset + 4`
		done
		echo "-------------------------------------"
		# example set/get as needed
		# devmem 0xF10C00B4 32 0x00002C5F
		# devmem 0xF10C00B4
	fi
}

restart_acsd()
{
	print_to_wifi_log "<$FUNCNAME>"
	killall -q -9 acsd2 2 >/dev/null
        nvram set acsd2_started=0
        acsd2
	nvram set acsd2_started=1
}

# Launch acsd2 once for RDK
# Param $1="1" if add_wlifs_to_brifs() and eapd is required to start acsd2
start_acsd2()
{
	print_to_wifi_log "<$FUNCNAME>";
	nvram unset wps_proc_status
        nvram unset acsd2_started

	# Wait up to 10s for bridge up
	bridge=`nvram get lan_ifname`
	if [ "$bridge" == "" ]; then
		print_to_wifi_log "<$FUNCNAME> ERROR: NO nvram lan_ifname, acsd2 NOT STARTED!";
		return
	fi
	wait=0
	timeout=10
	br_isup=`ifconfig $bridge | grep "UP"`
	while [ "$br_isup" == "" ] && [ $wait -lt $timeout ]; do
		br_isup=`ifconfig $bridge | grep "UP"`
		wait=$(( $wait + 1 ))
		sleep 1
	done
	print_to_wifi_log "<$FUNCNAME> waited:$wait s <= timeout:$timeout s for $bridge up";
	if [ $wait -ge $timeout ] && [ "$br_isup" == "" ]; then
		print_to_wifi_log "<$FUNCNAME> $bridge NOT up after $timeout s, acsd2 NOT STARTED!";
		return
	fi

	# Stop acsd2 if necessary
	do_app_stop acsd2 -9

	# If $1 = 1 then add wlifs to bridge and start eapd
	if [ "$1" == "1" ]; then
		# To recv escan events, add wlifs to bridge and start eapd for the first time
		add_wlifs_to_brifs "primary_wlifs" "1" "1"
		restart_app eapd
	fi

	restart_app acsd2
	print_to_wifi_log "<$FUNCNAME> acsd2 init done.";
}

# input: Index number of AP, value range from 0 to 23
# output: Name of the AP, eg. wl0.2, wl2.3 and wl2.7 etc.
get_ap_name_by_index()
{
	apIndex=$*

	if [ $apIndex -lt 16 ]; then
		x=$((apIndex % 2))
		y=$((apIndex / 2))
	else
		x=$((apIndex / MAX_NUM_BSS_PER_RADIO))
		y=$((apIndex % MAX_NUM_BSS_PER_RADIO))
	fi

	if [ $y == 0 ]; then
		echo "wl${x}"
	else
		echo "wl${x}.${y}"
	fi
}

# input: Index numbe of Radio, value from 0 to (max_number_of_radio -1)
# ouput: Name of the radio, eg wl0, wl1 and wl2
get_radio_name_by_index()
{
	radioIndex=$*
	echo wl${radioIndex}
}

# input: name of AP (wl0.1, wl1.6, wl2.7 etc)
# output: Index of the AP
get_ap_index_by_name()
{
	apName=$*

	x=$(echo ${apName} |awk '{ split($1, a, "."); print a[1]}')
	y=$(echo ${apName} |awk '{ split($1, a, "."); print a[2]}')

	x=$(echo ${x} |egrep -o '[0-9]+')
	[ ! -z ${y} ] && y=${y} || y=0

	if [ ${x} -lt 2 ]; then
		apIndex=$((y * 2 + x))
	else
		apIndex=$((x * MAX_NUM_BSS_PER_RADIO + y))
	fi
	echo ${apIndex}
}

# intput: name of radio (wl0, wl1, wl2)
# output: index of the Radio
get_radio_index_by_name()
{
	radioName=$*

	x=$(echo ${radioName} |awk '{ split($1, a, "."); print a[1]}')
	radioIndex=$(echo ${x} |egrep -o '[0-9]+')
	echo ${radioIndex}
}

# input: apIndex
# output: index of the Radio
get_radio_index_from_apIndex()
{
	apIndex=$*
	if [ ${apIndex} -lt 16 ]; then
		radioIndex=$((apIndex % 2))
	else
		radioIndex=$((apIndex / MAX_NUM_BSS_PER_RADIO))
	fi
	echo $radioIndex
}

# input: radioIndex
# output: band
get_band_from_radioIndex()
{
	radioIndex=$*
	radioName=$(get_radio_name_by_index $radioIndex)
	band=`wl -i $radioName band`
	echo ${band}
}

#input: apIndex
#output: band
get_band_from_apIndex()
{
	apIndex=$*
	radioIndex=$(get_radio_index_from_apIndex $apIndex)
	band=$(get_band_from_radioIndex $radioIndex)
	echo $band
}

# input: apIndex
# output: 1: primary interface 0: virtual interface
is_ap_primary()
{
	apIndex=$*
	if [ $apIndex -lt 16 ]; then
		y=$((apIndex / 2))
	else
		y=$((apIndex % MAX_NUM_BSS_PER_RADIO))
	fi

	if [ $y == 0 ]; then
		echo 1
	else
		echo 0
	fi
}

# input: apInex
# output: name of radio interface, eg wl0, wl1 or wl2
get_radio_name_by_ap_index()
{
	apIndex=$*
	if [ $apIndex -lt 16 ]; then
		x=$((apIndex % 2))
	else
		x=$((apIndex / MAX_NUM_BSS_PER_RADIO))
	fi
	echo wl${x}
}

getlanXifnames()
{
	lan_ifnames=`nvram get lan_ifnames`
	for i in $(seq 1 ${MAX_NUM_BRIDGE})
	do
		lanX_ifnames+=`nvram get lan${i}_ifnames`
	done
	echo "${lan_ifnames} ${lanX_ifnames}"
}

add_ifaces_to_flowmgr()
{
	print_to_wifi_log "<$FUNCNAME>"

	# Add the wl interfaces to the bridges.
	if [ "$(is_one_wifi)" == "1" ]; then
		add_wlifs_to_brifs "" "0" "1"
	else
		add_wlifs_to_brifs "" "1" "1"
	fi
}
