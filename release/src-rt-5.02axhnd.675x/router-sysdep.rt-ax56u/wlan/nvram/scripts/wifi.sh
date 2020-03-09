#!/bin/bash
mod='wfd bcm_pcie_hcd'

wl_down()
{
    ifstr="$(ifconfig -a | grep wl)"
    for wlx in $ifstr
    do
    if case $wlx in wl*) true;; *) false;; esac; then
        wl -i $wlx down
    fi
    done
}

case $1 in
suspend)
    wl_down
    killall -q dhd_monitor
    killall -q debug_monitor
    killall -q acsd acsd2
    acs_cli -i wl0 acs_suspend 1
	/rom/etc/init.d/bcm-wlan-drivers.sh stop
 	for m in ${mod//-/_}; do rmmod -w $m; done
 	;;
resume)
	grep -e${mod// /.ko -e}.ko /rom/etc/init.d/bcm-base-drivers.sh | sh
	/rom/etc/init.d/bcm-wlan-drivers.sh start
 	nvram restart
 	;;
modules)
	for m in ${mod//-/_}; do grep -e"^\<$m\>" /proc/modules; done
	;;
esac
