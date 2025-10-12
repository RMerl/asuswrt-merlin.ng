#!/bin/sh

# Create if need to debug wifi_setup.sh itself
# Use -f to force execution
SKIP_WIFI_SETUP="/data/SKIP_WIFI_SETUP"
if [[ -e $SKIP_WIFI_SETUP ]]; then
	if [[ $1 == "-f" ]]; then
		echo "wifi_setup.sh: detected presence of force -f"
		shift 1
	else
		echo "wifi_setup.sh: exiting detected presence of ${SKIP_WIFI_SETUP}"
		exit 0
	fi
fi

_CMD_ARGS_="$@"
_CALLER_=`cat /proc/$PPID/comm`
echo "$0 $_CMD_ARGS_ [$_CALLER_/$PPID]"

PHASE1_SEPARATE_NVRAM=1
PHASE2_SEPARATE_RC=1
export PATH=/usr/local/sbin:/usr/local/bin:/usr/local/usr/sbin:/usr/local/usr/bin:/usr/sbin:/usr/bin:/sbin:/bin
export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib/public:/usr/local/lib/private:/usr/local/lib/gpl:/lib:/usr/local/usr/lib
export LD_LIBRARY_PATH=/usr/lib64:/lib64:$LD_LIBRARY_PATH

# Source the wifi utility functions.
. $(dirname "$0")/wifi_util.sh
# source nvram utility functions
. $(dirname "$0")/nvram_util.sh

# Default values
WIFI_FEATURE_FILE=/tmp/wifi_setup_feature.txt
WIFI_IFNAMES="wl0 wl1 wl2"
WIFI_GUI_DIR=/usr/local/www
WPS_GPIO_MODS_DIR=/lib/modules/`uname -r`/kernel/drivers/bcm_media_gw/wps
WLAN_APPS_LOG_FILE="/rdklogs/logs/wifi_vendor_apps.log"
WIFI_READY_INDICATION_FILE="/tmp/.brcm_wifi_ready"
WIFI_VAP_READY_INDICATION_FILE="/tmp/.brcm_wifi_vap_ready"
BCACPE_WIFI_INDICATION_FILE="/tmp/.brcm_bcacpe_wifi"
if [[ -e $BCACPE_WIFI_INDICATION_FILE ]]; then
	BCACPE_RDKB_BUILD=1
else
	BCACPE_RDKB_BUILD=0
fi
WIFI_STARTED_INDICATION_FILE="/tmp/.brcm_wifi_started"
ONE_WIFI_BUILD_FILE="/tmp/.onewifi"
BOARD_HW_NVRAM_FILE="/data/board_hw_nvram"
APPS_DEFAULT_FILE="/usr/local/etc/wlan/nvram_default_apps.txt"
RADIO_DEFAULT_FILE="/usr/local/etc/wlan/nvram_default_radio.txt"
AP_DEFAULT_FILE="/usr/local/etc/wlan/nvram_default_ap.txt"
FACTORY_RESTORE_REQUESTED="/data/.factory_restore_requested"
SYSTEM_REBOOT_REQUESTED="/tmp/system_reboot_requested"

MAX_NUM_BRIDGE=32
MAX_NUM_VIFS=7
MAX_NUM_RADIOS=`echo $WIFI_IFNAMES | wc -w`
MAX_NUM_BSS_PER_RADIO=$((MAX_NUM_VIFS + 1))

# Detect the platform and set the defaults/paths accordingly.
RDK=$(is_rdk)
lan_ifname=$(get_bridge_name)
if [ "$RDK" == "1" ]; then
	lan_ipaddr="10.0.0.1"
	wifi_mods_dir=/lib/modules/`uname -r`
else
	lan_ipaddr="192.168.0.1"
	if [[ -d "/usr/local/lib/modules/`uname -r`/extra" ]]; then
		wifi_mods_dir=/usr/local/lib/modules/`uname -r`/extra
	else
		wifi_mods_dir=/lib/modules/`uname -r`/
	fi
fi

restore_defaults()
{
	print_to_wifi_log "<$FUNCNAME> $@"

	#for BCACPE_BUILD, first populate nvram from nvram default file
	#the priority order wlconf default< cmsMDM <default nvram file<
	#so here we need to move the do_wl_config to the end

	if [[ $BCACPE_RDKB_BUILD -ne 1 ]]; then
		# Populate the code defaults
		do_wl_config $(get_primary_ifnames $WIFI_IFNAMES)
		rc=$?
		print_to_wifi_log "<$FUNCNAME>  do_wl_config done, $rc"
	fi
	# Load product-specific nvram here.
	if [ "${PHASE2_SEPARATE_RC}" == "1" ]; then
		load_factory_nvram
		rc=$?
		print_to_wifi_log "load_factory_nvram, rc=$rc"
	fi

	if [ -e $APPS_DEFAULT_FILE ]; then
		factory_reset_apps
	fi
	if [ -e $AP_DEFAULT_FILE ]; then
		factory_reset_all_aps
	fi
	if [ -e $RADIO_DEFAULT_FILE ]; then
		factory_reset_all_radios
	fi
	if [ -e $SYSTEM_REBOOT_REQUESTED ]; then
		nvram commit
		reboot
	fi
	# for BCACPE_RDK_BUILD,
	# wlssk will take over from here to continue restore default, before calling wlconf
	# wlssk use existing nvram values such as bss_enabled to construct necessary
	# nvrams such as wlx_vifs etc.so no need of wlx_vifs in default configuration file.
	echo "restore_defaults: finished"
}

init_nvram()
{
	print_to_wifi_log "<$FUNCNAME> $@"
	nvram load -d /data/nvram
	rc=$?
	if [ $rc != 0 ]; then
		print_to_wifi_log "load /data/nvram failed with error code $(expr $rc - 256)"
		nvram load -d /data/nvram_bak
		rc=$?
		if [ $rc != 0 ]; then
			print_to_wifi_log "restore nvram from /data/nvram_bak failed with error code $(expr $rc - 256)"
			rm /data/nvram_bak 2> /dev/null
			rm /data/nvram 2> /dev/null
			touch /data/nvram
			restore_defaults
		else
			print_to_wifi_log "retore nvram from /data/nvram_bak succeeded"
		fi
	else
		print_to_wifi_log "load /data/nvram succeeded"
		nvram commit_reqd_clear
	fi
}

factory_reset_cleanup()
{
	rm -f /data/nvram* 2> /dev/null
	rm -f /nvram/.bcmwifi_* 2> /dev/null
}

init_wifi()
{
	if [ -e "/etc/device.properties" ]; then
		ONE_WIFI_BUILD="`cat /etc/device.properties | grep "OneWiFiEnabled" | awk -F '=' '{print $2}'`"
		if [ "$ONE_WIFI_BUILD" == "true" ]; then
			touch $ONE_WIFI_BUILD_FILE
		fi
	fi
	print_to_wifi_log "<$FUNCNAME> $@"

	apply_nvram_default=0
	if [ -e $FACTORY_RESTORE_REQUESTED ] || ([ ! -s "/data/nvram" ] && [ ! -e "/data/nvram_bak" ]); then
		apply_nvram_default=1
		rm -f $FACTORY_RESTORE_REQUESTED 2> /dev/null
		factory_reset_cleanup

		if [ "${PHASE2_SEPARATE_RC}" == "0" ]; then
			# update lattice-dma-wifi.xml if /data/nvram is not present
			if [ -e "/usr/local/etc/lattice/lattice-dm-wifi.xml" ]; then
				cp -f /usr/local/etc/lattice/lattice-dm-wifi.xml /data/
			fi
		fi
	fi

	# Check and copy lattice-dm-wifi.xml to /data
	if [ "${PHASE2_SEPARATE_RC}" == "0" ]; then
		if [ ! -e "/data/lattice-dm-wifi.xml" ] && [ -e "/usr/local/etc/lattice/lattice-dm-wifi.xml" ]; then
			cp -f /usr/local/etc/lattice/lattice-dm-wifi.xml /data/
		fi
	fi

	# OpenBFC RDK log
	if [ -d /rdklogs/logs ]; then
		if [ ! -f /rdklogs/logs/wifi_vendor.log ]; then
			touch /rdklogs/logs/wifi_vendor.log
		fi
		tail -f /var/log/user | grep "BRCM-WIFI" > /rdklogs/logs/wifi_vendor.log &
	fi

	# Load wifi driver kernel modules, based on packages
	if [ -z "$(lsmod | grep -w ^wlcsm)" ]; then
		insmod $wifi_mods_dir/wlcsm.ko

		# Load the board-specific default text files if necessary.
		if [ -e $BOARD_HW_NVRAM_FILE ]; then
			nvram load -t $BOARD_HW_NVRAM_FILE
			print_to_wifi_log "Loading board specific hw nvram parameters from $BOARD_HW_NVRAM_FILE"
		fi

		# Load the existing nvram settings before inserting the other kernel modules,
		# so the kernel module can access nvram.
		if [ "${apply_nvram_default}" == "0" ]; then
			init_nvram
		fi
	fi

	if [ -z "$(lsmod | grep -w ^wps_gpio)" ]; then
		insmod $WPS_GPIO_MODS_DIR/wps_gpio.ko
	fi

	if [ -e $wifi_mods_dir/hnd.ko ] && [ -z "$(lsmod | grep -w ^hnd)" ]; then
		insmod $wifi_mods_dir/hnd.ko
	fi

	if [ -e $wifi_mods_dir/emf.ko ] && [ -z "$(lsmod | grep -w ^emf)" ]; then
		insmod $wifi_mods_dir/emf.ko
	fi

	if [ -e $wifi_mods_dir/igs.ko ] && [ -z "$(lsmod | grep -w ^igs)" ]; then
		insmod $wifi_mods_dir/igs.ko
	fi

	if [ -e $wifi_mods_dir/wlerouter.ko ] && [ -z "$(lsmod | grep -w ^wlerouter)" ]; then
		insmod $wifi_mods_dir/wlerouter.ko
	fi

	if [ -e $wifi_mods_dir/wl.ko ] && [ -z "$(lsmod | grep -w ^wl)" ]; then
		insmod $wifi_mods_dir/wl.ko instance_base=$(get_wl_instances $WIFI_IFNAMES)
	fi

	if [ -e $wifi_mods_dir/dhd.ko ] && [ -z "$(lsmod | grep -w ^dhd)" ]; then
		insmod $wifi_mods_dir/dhd.ko instance_base=$(get_wl_instances $WIFI_IFNAMES) dhd_if_threshold=12288
	fi

	if [ -e $wifi_mods_dir/rdk_nlhal.ko ] && [ -z "$(lsmod | grep -w ^rdk_nlhal)" ]; then
		insmod $wifi_mods_dir/rdk_nlhal.ko
	fi

	# Set WL instance number after all interfaces are created.
	echo "$(get_wl_instances $WIFI_IFNAMES)" > /tmp/wl_instances.txt

	if [ "${apply_nvram_default}" == "1" ]; then
		print_to_wifi_log "factory reset, both /data/nvram and /data/nvram_bak have been removed"
		restore_defaults
	fi

#ifdef PHASE1_SEPARATE_NVRAM
if [ "${PHASE1_SEPARATE_NVRAM}" == "1" ]; then
	# Initialize lan_ifname nvram if not present.
	value=`nvram get lan_ifname`
	if [ "$value" == "" ]; then
		# The lan_ifname is not present in nvram, use the default above.
		nvram set lan_ifname="$lan_ifname"
	else
		lan_ifname="$value"
	fi

	# Set wifi_ifnames by actual wifi interfaces after insmoding drivers.
	wifi_ifnames=$(get_primary_ifnames $WIFI_IFNAMES)

	# Initialize lan_ifnames, which may contain primary and virtual IFs.
	lan_ifnames=`nvram get lan_ifnames`
	if [ "$lan_ifnames" == "" ]; then
		# Use the detected IFs to populate the nvram.
		lan_ifnames="$wifi_ifnames"
		nvram set lan_ifnames="$lan_ifnames"
	fi

	# Initialize ure_disable if it is not present.
	if [ "`nvram get ure_disable`" == "" ]; then
		nvram set ure_disable=1
	fi
fi
#endif /* PHASE1_SEPARATE_NVRAM */

#ifdef PHASE2_SEPARATE_RC
if [ "${PHASE2_SEPARATE_RC}" == "1" ]; then
	# Configure/create wlX.Y interfaces
	do_wl_config $wifi_ifnames
fi
#endif /* PHASE2_SEPARATE_RC */

	# Disable ipv6 for all wifi interfaces for RDK platforms
	if [ "$RDK" == "1" ]; then
		do_disable_ipv6 $wifi_ifnames
	fi

	#Add hotplug script for wds
	if [ -f /usr/local/sbin/bcm_dwds_hot_plug.sh ]; then
		cp /usr/local/sbin/bcm_dwds_hot_plug.sh /tmp/hotplug_wifi
	else
		# on XB7, file location is different
		cp /usr/bin/bcm_dwds_hot_plug.sh /tmp/hotplug_wifi
	fi
	chmod ugo+x /tmp/hotplug_wifi
	echo "/tmp/hotplug_wifi" >> /proc/sys/kernel/hotplug

	# Enable as needed for pcie debug
	# pcie_dumpregs "wl0" 0xAC 16
	# pcie_dumpregs "wl1" 0xAC 16

	# Create a VAP ready indication file to synchronize with vlan_util_tchxb6.sh script
	touch $WIFI_VAP_READY_INDICATION_FILE
}

start_wifi()
{
	print_to_wifi_log "<$FUNCNAME> $@"
#ifdef PHASE2_SEPARATE_RC
	# Use values from nvram
	lan_ifname=`nvram get lan_ifname`
	lan_ifnames=`nvram get lan_ifnames`
	wifi_ifnames=$(get_primary_ifnames $lan_ifnames)

if [ "$RDK" != "1" ]; then
	my_ipaddr=`ifconfig ${lan_ifname} | grep "inet addr" | cut -d ":" -f 2 | cut -d " " -f 1`
	if [ -z "$my_ipaddr" ]; then
		my_ipaddr=${lan_ipaddr}
	fi
	nvram set lan_ipaddr=${my_ipaddr}

	netmask=`ifconfig ${lan_ifname} | grep Bcast | cut -d ":" -f 4`
	if [ -z "$netmask" ]; then
		netmask="255.255.255.0"
	fi
	nvram set lan_netmask=${netmask}

	if [ "`nvram get lan_proto`" == "" ]; then
		nvram set lan_proto=static
	fi

	# Start httpd_wlan(port 8080)
	if [ "`nvram get httpd_wlan_port`" == "" ]; then
		nvram set httpd_wlan_port=8000
	fi
	if [ -z "$(ps -eaf | grep httpd_wlan | grep -v grep)" ]; then
		if [ -e /www/radio.asp ]; then
			WIFI_GUI_DIR=/www
		fi
		cd ${WIFI_GUI_DIR}; httpd_wlan; cd -
	fi

	# Start telnted and ushd
	telnet_en=`nvram get telnet_enable`
	if [ "${telnet_en}" == "1" ]; then
		if [ ! -z "$(ps -eaf | grep telnetd | grep -v grep)" ]; then
			killall telnetd
		fi

		# telnetd based on nvram lan_ipaddr
		telnetd -b `nvram get lan_ipaddr` -l /bin/sh

		# UTF related util
		ushd -i $lan_ifname -d

		# For OpenBFC Generic 18.2 or later - New Bridge design accessible through CM gateway.
		echo "echo filter_port add 4700 6 > /proc/driver/rtf/cmd" >> /tmp/lattice_generic.sh
		echo "echo filter_port add 23 6 > /proc/driver/rtf/cmd" >> /tmp/lattice_generic.sh
	fi

	# For OpenBFC Generic 18.2 or later - New Bridge design accessible through CM gateway.
	# For WPS monitor receiving UPnP packets on TCP port 1990
	echo "echo filter_port add 1990 6 > /proc/driver/rtf/cmd" >> /tmp/lattice_generic.sh
fi

if [ "${PHASE2_SEPARATE_RC}" == "1" ]; then
	if [ ! -e $WIFI_READY_INDICATION_FILE ]; then
		launch_permanent_apps
	fi
	if [ "$RDK" = "0" ] && [ ! -e $WIFI_READY_INDICATION_FILE ]; then
		touch $WIFI_READY_INDICATION_FILE
	fi
	if [ -e $WIFI_READY_INDICATION_FILE ]; then
		do_apps_start $wifi_ifnames

		# Add the interfaces to bridge and flow manager.
		if [ "$(is_one_wifi)" == "1" ]; then
			add_wlifs_to_brifs "" "0" "1"
		else
			add_wlifs_to_brifs "" "1" "1"
		fi

		# Start wifi interfaces
		do_wl_start $wifi_ifnames

		# On Generic, bridge is ready at this moment, so start emf
		emf_start $wifi_ifnames
		emf_add_iface $wifi_ifnames
	else
		touch $WIFI_READY_INDICATION_FILE
	fi
else
	# Launch wlmngr2 daemon
	if [ -z "$(ps | grep wlmngr2 | grep -v grep)" ]; then
		# For now, wifi hal needs wlmngr2
		rm -rf /var/wps.lock
		wlmngr2 &
	fi
fi
#endif /* PHASE2_SEPARATE_RC */

if [ "`nvram get sw_hash_clash`" == "1" ]; then
	# Enable hashclash for peak throughput testing between LAN and WiFi
	# when aggregation of Ethernet ports are used.
	echo e 1 > /proc/driver/ethsw/hashclash
fi

#ifdef PHASE1_SEPARATE_NVRAM
if [ "${PHASE1_SEPARATE_NVRAM}" == "1" ]; then
	# The /data/nvram size is zero(that is, newly created), commit it.
	if [ ! -s /data/nvram ];  then
		# Wifi hal checks the /data/nvram size to continue.
		nvram commit
	fi
fi
#endif /* PHASE1_SEPARATE_NVRAM */
}

stop_wifi()
{
	print_to_wifi_log "<$FUNCNAME> $@"

	[ -z "$*" ] && do_clean_up=1
	wifi_ifnames=$(get_wifi_ifnames $@)

        # stop traffic going in and out off from wifi to host
	do_wlifs_down $wifi_ifnames

#ifdef PHASE2_SEPARATE_RC
if [ "${PHASE2_SEPARATE_RC}" == "1" ]; then
	# Use the nvram values
	do_wl_down $wifi_ifnames
	do_apps_stop force $wifi_ifnames

	if [ "${do_clean_up}" == "1" ]; then
		stop_permanent_apps
		# Remove all wl interfaces from the bridge and flow manager.
		if [[ $# -eq 1 ]]; then
			del_wlifs_from_brifs $wifi_ifnames
		else
			del_wlifs_from_brifs
		fi

		# kill telnetd based on $lan_ifname ip addr, avoid  telnetd on privbr if
		my_ipaddr=`nvram get lan_ipaddr`
		tmpbuf=`ps -ef | grep -v grep | grep $my_ipaddr | grep telnetd`
		pid_tnetd=`echo $tmpbuf | awk '{print $2}'`
		if [ -n $pid_tnetd ]; then
			kill -9 $pid_tnetd
		fi

		killall ushd
		killall httpd_wlan
		if [ -e $WIFI_READY_INDICATION_FILE ]; then
			rm $WIFI_READY_INDICATION_FILE
		fi
		if [ -e $WIFI_STARTED_INDICATION_FILE ]; then
			rm $WIFI_STARTED_INDICATION_FILE
		fi
	fi
	emf_stop $wifi_ifnames
else
	# kill wlmngr2 daemon
	if [ -n "$(ps | grep wlmngr2 | grep -v grep)" ]; then
	       # no effect due to missing sig handler in wlmngr2
               killall wlmngr2
	fi
fi
#endif /* PHASE2_SEPARATE_RC */
	# Delete all wl virtual interfaces.
	del_wlifs $wifi_ifnames
}

restart_wifi() {
	print_to_wifi_log "<$FUNCNAME> $@"

	if [ "${PHASE2_SEPARATE_RC}" == "1" ] && [ ! -e $WIFI_READY_INDICATION_FILE ]; then
		print_to_wifi_log "<$FUNCNAME> wifi not ready yet!\n"
		return
	fi

	if [[ $BCACPE_RDKB_BUILD -eq 1 && $# -ge 1  && $g_mlo_enabled -eq 1 ]]; then
		print_to_wifi_log "<$FUNCNAME> MLO on:do not restart $@ individually, ignore"
		#wifi interface maybe down due to individual interface manipulation. bringup
		#interfaces
		wlconf_mlo_forceup
		return
	fi

	wifi_ifnames=$(get_wifi_ifnames $@)

	print_to_wifi_log "<$FUNCNAME> on $wifi_ifnames"
	do_wl_mlo_down
	do_apps_stop $wifi_ifnames

	# Remove the wl interfaces from the bridges.
	if [[ $# -eq 1 ]]; then
		del_wlifs_from_brifs $wifi_ifnames
	else
		del_wlifs_from_brifs
	fi

	# Config wifi interfaces
	do_wl_config $wifi_ifnames

	# Start apps. Assume all app nvram changes will be set to drivers via IOCTL.
	do_apps_restart $wifi_ifnames

	# Add the wl interfaces to the bridges.
	if [ "$(is_one_wifi)" == "1" ]; then
		add_wlifs_to_brifs "" "0" "1"
	else
		add_wlifs_to_brifs "" "1" "1"
	fi

	# Start wifi interfaces
	do_wl_start $wifi_ifnames

	if [ "${PHASE2_SEPARATE_RC}" == "1" ]; then
		if [ ! -e $WIFI_STARTED_INDICATION_FILE ]; then
			touch $WIFI_STARTED_INDICATION_FILE

			# on RDK, bridge is ready at this moment, start emf
			emf_start $wifi_ifnames
		else
			emf_del_iface $wifi_ifnames
		fi

		emf_add_iface $wifi_ifnames
	fi

	do_wl_mlo_up
	print_to_wifi_log "<$FUNCNAME> $@ done"
}

disable_flowmgr()
{
# 3390: CMWIFI Flowmanager is disabled in the case of -lattice-bridge-wifi builds.
     wl_num=$(lspci | egrep '43c5|43c4|4365|4359|43a9|4331|4332|43a0|43a2|4360|43a1|43b1|43c3|4429|442a|442b|4493|4494|4495|6710|a8d1|a8db' -c)
     if [ $wl_num -eq 0 ]; then
         echo 0 > /proc/sys/net/flowmgr/enable
     fi
}

# netxl layer is used with WiFi drivers only when running cable lab certification
# and enabled by nvram cablelab_cert=1
get_ifid_to_netxl_map()
{
	name=$*
	radio_map=(0 1 2) # default mapping
	indx=`get_radio_index_by_name $name`
	echo `expr ${radio_map[${indx}]} \* 16`
}

add_wl_netxl()
{
	print_to_wifi_log "<$FUNCNAME> $@"

	echo "call rmmod rtf"
	rmmod rtf

	echo "call modprobe mdqm"
	modprobe mdqm

	echo "call modprobe netxl"
	modprobe netxl

	lan_ifnames=`nvram get lan_ifnames`
	bridge=`nvram get lan_ifname`

	for ifname in $lan_ifnames ; do
		echo "call brctl delif $bridge $ifname"
		brctl delif $bridge $ifname
	done

	for ifname in $lan_ifnames ; do
		echo "call ifconfig $ifname down"
		ifconfig $ifname down
	done

	for ifname in $lan_ifnames ; do
		echo "call echo inf del $ifname > /proc/driver/flowmgr/cmd"
		echo inf del $ifname > /proc/driver/flowmgr/cmd
	done

	netxl_lan_ifnames = ""
	for ifname in $lan_ifnames ; do
		netxl_mapid=`get_ifid_to_netxl_map $ifname`
		netxl_ifname="${ifname}xl"
		netxl_lan_ifnames="${netxl_lan_ifnames} ${netxl_ifname}"
		echo "call echo addif ${netxl_ifname} $ifname $netxl_mapid > /proc/driver/netxl/cmd"
		echo addif ${netxl_ifname} $ifname $netxl_mapid > /proc/driver/netxl/cmd
	done
	print_to_wifi_log "<$FUNCNAME>: addif ${netxl_lan_ifnames} and ${lan_ifnames} to netxl done"

	for ifname in $netxl_lan_ifnames ; do
		print_to_wifi_log "call brctl addif $bridge ${ifname}"
		brctl addif $bridge ${ifname}
	done

	for ifname in $netxl_lan_ifnames  ; do
		print_to_wifi_log "call ifconfig ${ifname} up"
		ifconfig ${ifname} up
	done

	for ifname in $lan_ifnames ; do
		print_to_wifi_log "call ifconfig $ifname up"
		ifconfig $ifname up
	done

	for ifname in $netxl_lan_ifnames ; do
		print_to_wifi_log "call echo inf add ${ifname} > /proc/driver/flowmgr/cmd"
		echo inf add ${ifname} > /proc/driver/flowmgr/cmd
	done

	print_to_wifi_log "call modprob rtf"
	modprobe rtf

	print_to_wifi_log "call echo host_inf $bridge > /proc/driver/rtf/cmd"
	echo host_inf $bridge > /proc/driver/rtf/cmd

	print_to_wifi_log "call echo enable 1 > /proc/driver/rtf/cmd"
	echo enable 1 > /proc/driver/rtf/cmd

	if [ "`pgrep debug_monitor`" != "" ]; then
		killall debug_monitor
	fi

	for ifname in $lan_ifnames ; do
		ifconfig $ifname down
	done

	for ifname in $lan_ifnames ; do
		print_to_wifi_log "stop hostapd for ${ifname}"
		pgrep -f /tmp/${ifname}_hapd.conf | xargs kill -9 2> /dev/null
		sleep 2
	done

	for ifname in $lan_ifnames ; do
		print_to_wifi_log "launch hostapd for ${ifname}"
		if [ "`nvram get hapd_dbg`" == "1" ]; then
			hostapd -ddt /tmp/${ifname}_hapd.conf &
		else
			hostapd -t /tmp/${ifname}_hapd.conf &
		fi
		sleep 2
	done
}

# Main routine
case "$1" in
	"init")
		init_wifi
		;;
	"start")
		if [ "$RDK" == "1" ] && [ ! -e "/tmp/wl_instances.txt" ]; then
			init_wifi
		fi
		start_wifi
		;;
	"stop")
		if [ "$(is_one_wifi)" == "0" ]; then
			if [ -e /data/nvram ];  then
				nvram commit
			else
				nvram commit_reqd_clear
			fi
		fi
                shift
		stop_wifi $@
		;;
	"restart")
#ifdef PHASE2_SEPARATE_RC
if [ "${PHASE2_SEPARATE_RC}" == "1" ]; then
		# restart designated radios specified as primary interface name
		shift
		restart_wifi $@
else
		echo "rc restart"
		rc restart
fi
#endif /* PHASE2_SEPARATE_RC */
		if [ "`nvram get cablelab_cert`" == "1" ]; then
			add_wl_netxl
		fi
		;;
	"restart_wifi_acsd")
		shift
		restart_wifi $@
		restart_acsd
		;;
	"stop_security_daemons")
if [ "${PHASE2_SEPARATE_RC}" == "1" ]; then
		# stop security daemons on designated radios specified as primary interface name
		shift
		stop_security_daemons $@
else
		# not support
		echo "$0 $1 not supported"
fi
		;;
	"start_security_daemons")
if [ "${PHASE2_SEPARATE_RC}" == "1" ]; then
		# start security daemons on designated radios specified as primary interface name
		shift
		start_security_daemons $@
else
		# not support
		echo "$0 $1 not supported"
fi
		;;
	"restart_security_daemons")
		# So stop/start security deamons run in the same context,
		# for avoid interleave stop/start security daemons
		#
		# TODO: NOT support restart security daemons for multiple radios
		# in one shot
		shift
		restart_security_daemons $@
		;;
	"disable_flowmgr")
		disable_flowmgr
		;;
	"pcie_dumpregs")
		echo "Usage: pcie_dumpregs <wlX hexoffset numreg>"
		ifnames=$(get_primary_ifnames $WIFI_IFNAMES)
		offset=0
		numreg=16
		if [ $# -gt 1 ]; then
			ifnames=$2
		fi
		if [ $# -gt 2 ]; then
			offset=$3
		fi
		if [ $# -gt 3 ]; then
			numreg=$4
		fi
		echo "$ifnames $offset $numreg"
		for i in $ifnames
		do
			paddr=$(pcie_baseaddr $i)
			printf '%s: 0x%08x+0x%04x %d\n' $i $((paddr)) $offset $((numreg))
			pcie_dumpregs_from_offset $i $offset $numreg
		done
		;;
        "enable_wifi")
                shift
                do_wl_start $@
                launch_wifi_apps
                ;;
        "setup_ifaces")
                shift
                do_wl_up $@
                do_ifconfig_wlifs_up $@
                ;;
        "start_wifi_apps")
                shift
                launch_wifi_apps
                ;;
	"reset_ap_nvram")
		shift
		reset_ap_nvram_by_names $@
		nvram commit
		;;
	"reset_radio_nvram")
		shift
		reset_radio_nvram_by_names $@
		nvram commit
		;;
	"factory_reset_all_aps")
		factory_reset_all_aps
		nvram commit
		;;
	"factory_reset_all_radios")
		factory_reset_all_radios
		nvram commit
		;;
	"factory_reset_apps")
		factory_reset_apps
		nvram commit
		;;
	"launch_permanent_apps")
		launch_permanent_apps
		;;
	"restore_defaults")
		restore_defaults
		;;
	"add_ifaces_to_flowmgr")
		print_to_wifi_log "<$FUNCNAME> start add_ifaces_to_flowmgr"
		add_ifaces_to_flowmgr
		;;
	"erase")
		 if [[ $BCACPE_RDKB_BUILD -eq 1 ]]; then
			 rm  -f /data/psi_wifi
			 nvram unset _default_restored_
		 else
			if [ $PHASE2_SEPARATE_RC == "0" ]; then
				rm -f /data/lattice-dm-wifi.xml
			fi
			touch $FACTORY_RESTORE_REQUESTED
		fi
		factory_reset_cleanup
		nvram commit_reqd_clear
		;;
	"wifi_cfg_erase")
		if [[ $BCACPE_RDKB_BUILD -eq 1 ]]; then
			stop_permanent_apps
			nvram unset _default_restored_
			killall -9 wlssk
			killall wifi_rdk_initd
			rm  -f $WIFI_READY_INDICATION_FILE
			rm  -f /data/psi_wifi
			rm  -f /tmp/nvm_staged
			wifi_rdk_initd
		else
			if [ $PHASE2_SEPARATE_RC == "0" ]; then
				rm -f /data/lattice-dm-wifi.xml
			fi
			rm -f /data/nvram*
		fi
		nvram commit_reqd_clear
		;;
	"add_wl_netxl")
		if [ "`nvram get cablelab_cert`" == "1" ]; then
			add_wl_netxl
		fi
		;;
esac

if [[ ! -f $WIFI_FEATURE_FILE ]]; then
	echo "launch_permanent_apps" > $WIFI_FEATURE_FILE
	echo "wifi_cfg_erase" >> $WIFI_FEATURE_FILE
fi
echo "$0 $_CMD_ARGS_ done [$_CALLER_/$PPID]!"
exit 0
