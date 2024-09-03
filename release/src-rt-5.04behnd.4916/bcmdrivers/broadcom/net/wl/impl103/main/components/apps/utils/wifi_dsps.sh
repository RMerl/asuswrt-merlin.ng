#!/bin/sh
#
# WiFi Deep Sleep Power Save
# wifi_dsps.sh <WiFi interface name> <power_down|power_up>
#
# Copyright (C) 2024, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Dual>>
#
# $Id$

wl0_ifname=`nvram get wl0_ifname`
wl1_ifname=`nvram get wl1_ifname`
wl2_ifname=`nvram get wl2_ifname`
wl3_ifname=`nvram get wl3_ifname`

show_help () {
	echo "Syntax: $0 <WiFi interface name> <power_down|power_up>"
	exit
}

# Help/command use error
if [[ $# -eq 1 ]] && [[ $1 == "--help" ]]; then
	show_help
	exit 0
elif [[ $# -ne 2 ]]; then
	echo "Invalid number of parameters! $#"
	show_help
	exit 0
fi

# Argument 1 is WiFi interface
# Argument 2 is operation: poweroff or poweron
ifname=$1
operation=$2

if [[ $(expr match "$ifname" 'wl') == 0 ]] && [[ $(expr match "$ifname" 'eth') == 0 ]]; then
	show_help
	exit 0
fi

if [[ $operation != "power_up" ]] &&
   [[ $operation != "power_down" ]] ; then
	show_help
	exit 0
fi

#Script/Platform customization
#For CMWIFI platforms:
#       device tree/linux control voltage regulators
#       no user level programming required.
#For BCARouter platforms:
#       device tree/linux control voltage regulators is TBD
#       use below customization to add user level control
#       e.g.
#       4912 reworked reference -
#       gpio 5 controls pcie0
#       gpio 4 controls pcie1, pcie2
#       no gpio control/radio connected pcie3
#       Note: gpio is reserved for alternate purpose,
#       the export of gpio will fail

#__init
__init()
{
	wifi_list="wl0 wl1 wl2 wl3";
	model=`cat /sys/firmware/devicetree/base/model`
	gpio_num_pci0=NA
	gpio_num_pci1=NA
	gpio_num_pci2=NA
	gpio_num_pci3=NA
	WIFI_DSPS_INFO_FILE="/tmp/.wifi_dsps_info"
	DM_DIR=/tmp/dm
	if [[ -d /sys/devices/platform/ubus-bus ]]; then
		PCIE_NET_BASE=/sys/devices/platform/ubus-bus
		PCIE_NODE_DEPTH=6
		PCIE_BUS_DEPTH=7
	else
		PCIE_NET_BASE=/sys/devices/platform
		PCIE_NODE_DEPTH=5
		PCIE_BUS_DEPTH=6
	fi
	PCIE_DEV_PATH=${PCIE_NET_BASE}/*pcie/pci*/*/*/net
	VPCIE_DEV_PATH=${PCIE_NET_BASE}/*vpcie/pci*/*/net

	if [[ $(expr match "$model" 'Broadcom-v8A') != 0 ]]; then
		model=BCARouter
		board=`cat /sys/firmware/devicetree/base/uboot_env | grep board | cut -d= -f2`
		BRCM_PCI_DRV=bcm-pcie
		wl_mlo_config=`nvram kget wl_mlo_config`
	elif [[ $(expr match "$model" '339') != 0 ]]; then
		model=CMWIFI
		PS_OPTIONS="-aef"
		BRCM_PCI_DRV=brcm-pcie
		if  [ ! -e "/sys/bus/platform/drivers/${BRCM_PCI_DRV}" ] ; then
		# 3390/3392/33940/33941 releases have driver path exported as brcm-pci
			BRCM_PCI_DRV=brcm-pci
		fi
		wl_mlo_config=`nvram get wl_mlo_config`
	else
		#echo "Warning:model=${model}, assuming BCARouter platform"
		model=BCARouter
		BRCM_PCI_DRV=bcm-pcie
		wl_mlo_config=`nvram kget wl_mlo_config`
	fi

	if  [ ! -e "${WIFI_DSPS_INFO_FILE}" ] ; then
		echo "model=$model" > ${WIFI_DSPS_INFO_FILE}
		wl0_gpio=$(get_gpio_num $wl0_ifname)
		wl1_gpio=$(get_gpio_num $wl1_ifname)
		wl2_gpio=$(get_gpio_num $wl2_ifname)
		wl3_gpio=$(get_gpio_num $wl3_ifname)
		# cache gpio mapping for future use
		echo "GPIO for ${wl0_ifname}=${wl0_gpio}" >> ${WIFI_DSPS_INFO_FILE}
		echo "GPIO for ${wl1_ifname}=${wl1_gpio}" >> ${WIFI_DSPS_INFO_FILE}
		echo "GPIO for ${wl2_ifname}=${wl2_gpio}" >> ${WIFI_DSPS_INFO_FILE}
		echo "GPIO for ${wl3_ifname}=${wl3_gpio}" >> ${WIFI_DSPS_INFO_FILE}
	fi

	# Radio powerup/down with MLO overrides support
	if [[ ! -z "$wl_mlo_config" ]] ; then
		mlo_wl0=`echo ${wl_mlo_config} | cut -d " " -f 1`
		[[ "$mlo_wl0" -eq "0" ]] && mlo_map_if="wl0"

		mlo_wl1=`echo ${wl_mlo_config} | cut -d " " -f 2`
		[[ "$mlo_wl1" -eq "0" ]] && mlo_map_if="wl1"

		mlo_wl2=`echo ${wl_mlo_config} | cut -d " " -f 3`
		[[ "$mlo_wl2" -eq "0" ]] && mlo_map_if="wl2"

		mlo_wl3=`echo ${wl_mlo_config} | cut -d " " -f 4`
		[[ "$mlo_wl3" -eq "0" ]] && mlo_map_if="wl3"
	else
		mlo_wl0="-1"; mlo_wl1="-1"; mlo_wl2="-1"; mlo_wl3="-1";
		mlo_map_if="-1";
	fi
}

get_gpio_num()
{

	gpio=`cat /tmp/.wifi_dsps_info | grep GPIO | grep $1| cut -d= -f2`
	if [[ -z $gpio ]] ; then
		pci_bus=`ls -Ld ${PCIE_DEV_PATH}/*|grep ${1} \
			| cut -d '/' -f ${PCIE_BUS_DEPTH} | cut -d ':' -f 1  | sed s/000//`
		if [[ -z $pci_bus ]] ; then
			gpio=NA
		elif [[ $pci_bus == "pci0" ]] ; then
			gpio=${gpio_num_pci0}
		elif [[ $pci_bus == "pci1" ]] ; then
			gpio=${gpio_num_pci1}
		elif [[ $pci_bus == "pci2" ]] ; then
			gpio=${gpio_num_pci2}
		elif [[ $pci_bus == "pci3" ]] ; then
			gpio=${gpio_num_pci3}
		fi
	fi
	echo $gpio
}

WIFI_DSPS_LOCK_FILE="/tmp/.wifi_dsps_lock"
acquire_lock() {

	while [[ -e ${WIFI_DSPS_LOCK_FILE} ]]
	do
		sleep 1
	done

	#busybox: touch <file> not available
	echo "Locking for $ifname" > ${WIFI_DSPS_LOCK_FILE}
}

release_lock() {
	rm -f $WIFI_DSPS_LOCK_FILE
}

get_wifirf_vreg()
{
	busid=`ls -Ld ${PCIE_DEV_PATH}/${1} | cut -d '/' -f ${PCIE_BUS_DEPTH} | cut -d ':' -f 1 | sed s/pci000//g`
	echo wifirf${busid}pwr
}

gen_wifirf_powerup_override()
{
	if [[ $(expr match "$model" 'CMWIFI') != 0 ]]; then
		wifirf=$(get_wifirf_vreg $ifname)
		echo "
			if [ -e "/proc/driver/wrfpwr/cmd" ]; then
				echo regpwrcntrl ${wifirf} enable > /proc/driver/wrfpwr/cmd;
			fi;
		"
	fi
}

gen_wifirf_powerdown_override()
{
	if [[ $(expr match "$model" 'CMWIFI') != 0 ]]; then
		wifirf=$(get_wifirf_vreg $ifname)
		echo "
			if [ -e "/proc/driver/wrfpwr/cmd" ]; then
				echo regpwrcntrl ${wifirf} disable > /proc/driver/wrfpwr/cmd;
			fi;
		"
	fi
}

get_pcie_devaddr()
{
	if [[ -e ${VPCIE_DEV_PATH}/${1} ]]; then
		devpath=${VPCIE_DEV_PATH}
	else
		devpath=${PCIE_DEV_PATH}
	fi
	devaddr=`ls -Ld $devpath/${1} | cut -d '/' -f ${PCIE_NODE_DEPTH}`
	echo $devaddr
}

gen_gpio_powerup_override()
{
	if [[ $(expr match "$model" 'BCARouter') != 0 ]]; then
		gpio_num=$(get_gpio_num $1)
		if [[ $gpio_num != "NA" ]]; then
			echo "
				if [ -e "/sys/class/gpio/gpio${gpio_num}/value" ]; then
					echo 1 >  /sys/class/gpio/gpio${gpio_num}/value;
				fi;
			"
		fi
	fi
}

gen_gpio_powerdown_override()
{
	gpio_num=$(get_gpio_num $1)
	COND="[[ TRUE ]] "
	if [[ $(expr match "$model" 'BCARouter') != 0 ]] &&
	   [[ $gpio_num != "NA" ]]; then
		if  [[ "$gpio_num" == $(get_gpio_num $wl0_ifname) ]]; then
			COND=" ${COND} && [[ -e /tmp/wifi_dsps_${wl0_ifname}.powerdown ]]"
		fi
		if  [[ "${gpio_num}" == $(get_gpio_num $wl1_ifname) ]]; then
			COND="${COND} && [[ -e /tmp/wifi_dsps_${wl1_ifname}.powerdown ]] "
		fi
		if  [[ "${gpio_num}" == $(get_gpio_num $wl2_ifname) ]]; then
			COND="${COND}  && [[ -e /tmp/wifi_dsps_${wl2_ifname}.powerdown ]]"
		fi
		echo "
			if ${COND} ; then
				echo ${gpio_num} > /sys/class/gpio/export;
				if [ -e "/sys/class/gpio/gpio${gpio_num}/value" ]; then
					echo out > /sys/class/gpio/gpio${gpio_num}/direction;
					echo 0 >  /sys/class/gpio/gpio${gpio_num}/value;
				fi
			fi;
		"
	fi
}
get_hostapd_pid()
{
	if [[ $1 == ${wl0_ifname} ]] ; then
		ifname="wl0"
	elif [[ $1 == ${wl1_ifname} ]] ; then
		ifname="wl1"
	elif [[ $1 == ${wl2_ifname} ]] ; then
		ifname="wl2"
	elif [[ $1 == ${wl3_ifname} ]] ; then
		ifname="wl3"
	fi

	for pid in `pidof hostapd`;
	do
		match=`cat /proc/${pid}/cmdline  | awk '{printf $1 " " }'|grep $ifname;`
		if [[ ! -z "${match}" ]] ; then
			echo $pid
		fi
	done
}

get_mlo_ap_config()
{
# no associative array support in busybox, use switch-case
	case "$1" in
		"wl0")
			echo ${mlo_wl0}
			;;
		"wl1")
			echo ${mlo_wl1}
			;;
		"wl2")
			echo ${mlo_wl2}
			;;
		"wl3")
			echo ${mlo_wl3}
			;;
	esac

}

#
# MLO has inter-ap dependency during power down, up operation for driver bind, radio config
# The script provide for execution hooks to handle inter-ap dependency
# pre_powerdown, post_powerdown, pre_powerup, post_powerup
#
gen_mlo_override_script()
{
	echo "
		if [[ \${1} == pre_powerdown ]] ; then
			wl -i ${mlo_map_if} mlo_down;
		fi;

		if [[ \${1} == post_powerdown ]] ; then
			echo;
		fi;

		if [[ \${1} == pre_powerup ]] ; then
			echo;
		fi;

		if [[ \${1} == post_powerup ]] ; then
	"
	# bring up MLO when all participating APs are synced (mlc_state: 6)
	for name in $wifi_list; do
		is_mlo=$(get_mlo_ap_config $name)
		if [[ ! "${is_mlo}" -eq "-1" ]]; then
			echo "
				for i in $(seq 1 3); do
					mlc_state=\`wl -i ${name} mlc_state | cut -d \" \" -f 1\`;
					if [[ \"\$mlc_state\" -eq \"6\" ]] ; then
						break;
					fi;
					sleep 1;
				done;
			"
		fi;
	done

	echo "
			wl -i ${mlo_map_if} mlo_up;
			wlconf ${mlo_map_if} configure_mlo;
		fi;
	"
}

mlo_pre_powerdown_override()
{
	for name in $wifi_list; do
		is_mlo=$(get_mlo_ap_config $name)
		if [[ ! "${is_mlo}" -eq "-1" ]] &&
			[[ -e "/tmp/wifi_dsps_${name}.powerdown" ]]; then
				# powerdown indicates override was already applied
				return;
		fi
	done
	sh ${DEBUG_SCR} /tmp/wifi_dsps_mlo.override pre_powerdown
}

mlo_post_powerdown_override()
{
	# no-op for now
	echo ;
}

mlo_pre_powerup_override()
{
	for name in $wifi_list; do
		is_mlo=$(get_mlo_ap_config $name)
		if [[ ! "${is_mlo}" -eq "-1" ]] &&
			[[ -e "/tmp/wifi_dsps_${name}.powerdown" ]] &&
			[[ ! -z `ls -d /sys/class/net/${name} 2> /dev/null` ]] ; then
				# presence devname would indicates override was already applied
				sh ${DEBUG_SCR} /tmp/wifi_dsps_mlo.override pre_powerup
		fi
	done
}

mlo_post_powerup_override()
{
	for name in $wifi_list; do
		is_mlo=$(get_mlo_ap_config $name)
		if [[ ! "${is_mlo}" -eq "-1" ]] &&
			[[ -e "/tmp/wifi_dsps_${name}.powerup" ]]; then
				# presence indicate not all participating MLO APs are up
				return;
		fi
	done
	sh ${DEBUG_SCR} /tmp/wifi_dsps_mlo.override post_powerup
}

gen_powerup_script()
{
	ifname=$1
	pid_hostapd=$(get_hostapd_pid ${ifname})
	if [[ ! -z ${pid_hostapd} ]]; then
		cmd_hostapd=`cat /proc/${pid_hostapd}/cmdline  | awk '{printf $1 " " }';`
	fi
	devaddr_node=$(get_pcie_devaddr $ifname)
	gen_gpio_powerup_override $ifname
	gen_wifirf_powerup_override $ifname
	echo "
		echo ${devaddr_node} > /sys/bus/platform/drivers/${BRCM_PCI_DRV}/bind;
	"
	if [[ "$model" == BCARouter ]]; then
		echo "wlaffinity apply powerup;"
	fi
	echo "
		wlconf ${ifname} up;
	"
	# bring up interface manually in some cases, e.g dfs fixed channel case
	echo "
		[[ -z \$(brctl show|grep ${ifname}) ]] && sleep 1;
		[[ -z \$(brctl show|grep ${ifname}) ]] && ifconfig ${ifname} up && sleep 1;
	"
	if [[ ! -z "${cmd_hostapd}" ]]; then
		echo "
			hostapd_cli -i ${ifname} enable;
			while true;
			do
				hapd_intf_state=\`hostapd_cli -i ${ifname} status | grep state | cut -d '=' -f 2\`;
				if [[ \"\${hapd_intf_state}\" == \"ENABLED\" ]] ||
					[[ \"\${hapd_intf_state}\" == \"DFS\" ]]; then
					break;
				fi;
				sleep 1;
			done;
		"
	fi

	pid_acsd2=`pidof acsd2`
	if [[ ! -z ${pid_acsd2} ]] ; then
		acs_cli_mode=`acs_cli2 -i ${ifname} mode|sed s/\:.*//`;
		echo "
			acs_cli2 -i ${ifname} mode ${acs_cli_mode};
			acs_cli2 -i ${ifname} acs_restart;
		"
	fi
	pid_escand=`pidof escand`
	if [[ ! -z ${pid_escand} ]] ; then
		es_cli_mode=`es -i ${ifname} mode|sed s/\:.*//`;
		echo "es -i ${ifname} mode ${es_cli_mode}";
	fi
}

gen_powerdown_script()
{
	ifname=$1
	wlif_list=`ifconfig -a  | grep -E 'wl|eth' | awk '{print $1}' | grep ${ifname}`
	pid_hostapd=$(get_hostapd_pid ${ifname})
	pid_acsd2=`pidof acsd2`
	pid_escand=`pidof escand`
	devaddr_node=$(get_pcie_devaddr $ifname)
	is_hwa_cap=`wl -i ${ifname} cap | grep hwa`
	[[ ! -z ${pid_acsd2} ]] && echo "acs_cli2 -i ${ifname} mode 0;"
	[[ ! -z ${pid_escand} ]] && echo "es -i ${ifname} mode 0;"
	[[ ! -z "${is_hwa_cap}" ]] && echo "wl -i ${ifname} hwa_rxpost_reclaim_en 1;"
	echo "
		wlconf ${ifname} down;
		for wlif in ${wlif_list}; do ifconfig \${wlif} down; done;
	"
	if [[ ! -z ${pid_hostapd} ]]; then
		if [[ -e ${DM_DIR}/${pid_hostapd} ]]; then
			rm -f ${DM_DIR}/${pid_hostapd}
		fi
		echo "hostapd_cli -i ${ifname} disable;"
	fi
	echo "
		wlaffinity save powerdown;
		echo ${devaddr_node} > /sys/bus/platform/drivers/${BRCM_PCI_DRV}/unbind;
	"
	gen_gpio_powerdown_override $ifname
	gen_wifirf_powerdown_override $ifname
}

#Locking:
# 1. Global lock for any power state operation across any device
#Status:
# 1. Existence of /tmp/wifi_dsps_${ifname}.powerdown indicates power down requested
# 2. Existence of /tmp/wifi_dsps_${ifname}.powerup indicates device in power down state
# 3. Removal of /tmp/wifi_dsps_${ifname}.powerdown indicates power up requested
# 4. Removal of /tmp/wifi_dsps_${ifname}.powerup indicates device powered up

#load, initialize script variables
__init;

#Start operation
acquire_lock;
	is_mlo_if=$(get_mlo_ap_config $ifname)
	if [[ ! "${is_mlo_if}" -eq "-1" ]] &&
	     [[ ! -e "/tmp/wifi_dsps_mlo.override" ]]; then
		mlo_override=$(gen_mlo_override_script)
		echo ${mlo_override} >> /tmp/wifi_dsps_mlo.override;
	fi

	if [[ $operation == "power_down" ]]; then
		if [ -e "/tmp/wifi_dsps_${ifname}.powerdown" ]; then
			echo "Radio $ifname already in power down state"
			release_lock;
			exit 0;
		fi
	elif [[ $operation == "power_up" ]]; then
		if [ ! -f "/tmp/wifi_dsps_${ifname}.powerup" ]; then
			echo "Radio $ifname already in power up state"
			release_lock;
			exit 0;
		fi
	fi

	if [[ $operation == "power_down" ]]; then
		# check if device is registered with pcie subsystem */
		devaddr=`ls -Ld ${PCIE_DEV_PATH}/${1} 2> /dev/null | cut -d '/' -f ${PCIE_NODE_DEPTH} | cut -d '.' -f 1`
		vdevaddr=`ls -Ld ${VPCIE_DEV_PATH}/${1} 2> /dev/null | cut -d '/' -f ${PCIE_NODE_DEPTH} | cut -d '.' -f 1`
		if [[ -z $devaddr ]] && [[ -z $vdevaddr ]]; then
			echo "Radio $ifname not initialized"
			release_lock;
			exit 0;
		fi

		[[ ! ${is_mlo_if} -eq "-1" ]] && mlo_pre_powerdown_override;

		powerup_cmd_seq=$(gen_powerup_script $ifname)
		powerdown_cmd_seq=$(gen_powerdown_script $ifname)
		#gen script : device is transitioning to powerdown
		echo ${powerdown_cmd_seq} >> /tmp/wifi_dsps_${ifname}.powerdown
		#execute script: transition to power down state
		sh ${DEBUG_SCR} /tmp/wifi_dsps_${ifname}.powerdown
		#gen script: device in power down
		echo ${powerup_cmd_seq} >> /tmp/wifi_dsps_${ifname}.powerup

		[[ ! ${is_mlo_if} -eq "-1" ]] && mlo_post_powerdown_override;
	fi

	if [[ $operation == "power_up" ]]; then

		[[ ! ${is_mlo_if} -eq "-1" ]] && mlo_pre_powerup_override;
		#rm script : device is transitioning to power up
		rm /tmp/wifi_dsps_${ifname}.powerdown
		#execute script: transition to power up state
		sh ${DEBUG_SCR} /tmp/wifi_dsps_${ifname}.powerup
		# wait for network interface to be registered
		while [[ -z `ls -d /sys/class/net/${ifname} 2> /dev/null` ]]
		do
			echo "wifi_dsps.sh: interface ${ifname} not added to network stack, sleep(1)"
			sleep 1
		done
		#rm script: device powered up/configured state
		rm /tmp/wifi_dsps_${ifname}.powerup

		[[ ! ${is_mlo_if} -eq "-1" ]] && mlo_post_powerup_override;
	fi

#Operation completed
release_lock;
exit
