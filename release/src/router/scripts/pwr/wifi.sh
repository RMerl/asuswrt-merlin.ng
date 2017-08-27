#!/bin/sh
mod='wl dhd wfd bcm63xx_pcie '

case $1 in
suspend)
	killall -q acsd
	for m in $mod; do rmmod -w $m; done
	;;
resume)
	grep -e${mod// /.ko -e}KERNELVER= /rom/etc/init.d/bcm-base-drivers.sh |sh
	nvram commit
	;;
esac
