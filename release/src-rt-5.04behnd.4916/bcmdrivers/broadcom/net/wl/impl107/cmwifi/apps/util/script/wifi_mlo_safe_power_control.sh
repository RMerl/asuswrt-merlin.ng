#!/bin/sh
#Use only for cases where wlconf <wlx> power_down/up doesnot support mlo config

if [[ "$#" != 2 ]]; then
	echo "$0 <wlx> power_down"
	echo "$0 <wlx> power_up"
	exit
fi

old_val=`nvram get wl_mlo_config_def`

if [[ "$old_val" == "" ]]; then
	old_val=`nvram get wl_mlo_config`
	nvram set wl_mlo_config_def="$old_val"
	echo "saving wl_mlo_config $old_val"
	nvram commit
fi

if [[ "$2" == "power_down" ]]; then
	if [[ -e "/tmp/wifi_dsps_wl0.powerdown" ]] ||
	   [[ -e "/tmp/wifi_dsps_wl1.powerdown" ]] ||
	   [[ -e "/tmp/wifi_dsps_wl2.powerdown" ]] ; then
		echo "Config is already transitioned to nonMLO";
		wlconf $1 power_down
		exit
	fi
	wl -i wl0 hwa_rxpost_reclaim_en 1
	wl -i wl1 hwa_rxpost_reclaim_en 1
	wl -i wl2 hwa_rxpost_reclaim_en 1
	wifi_setup.sh stop
	rmmod wl;
	rmmod dhd;
	rmmod igs;
	rmmod emf;
	rmmod hnd;
	nvram set wl_mlo_config="-1 -1 -1 -1";
	nvram commit
	wifi_setup.sh init
	wifi_setup.sh start
	wifi_setup.sh rg_network_bridged_add
	rc restart
	wlconf $1 power_down
fi

if [[ "$2" == "power_up" ]]; then
	wlconf $1 power_up

	if [[ -e "/tmp/wifi_dsps_wl0.powerdown" ]] ||
	   [[ -e "/tmp/wifi_dsps_wl1.powerdown" ]] ||
	   [[ -e "/tmp/wifi_dsps_wl2.powerdown" ]] ; then
		echo "Not all radio powered up, do not transiton to MLO config";
		exit
	fi
	wl -i wl0 hwa_rxpost_reclaim_en 1
	wl -i wl1 hwa_rxpost_reclaim_en 1
	wl -i wl2 hwa_rxpost_reclaim_en 1
	wifi_setup.sh stop
	rmmod wl;
	rmmod dhd;
	rmmod igs;
	rmmod emf;
	rmmod hnd;
	old_val=`nvram get wl_mlo_config_def`
	echo "restoring wl_mlo_config $old_val"
	nvram set wl_mlo_config="$old_val"
	nvram commit
	wifi_setup.sh init
	wifi_setup.sh start
	wifi_setup.sh rg_network_bridged_add
	rc restart
fi
