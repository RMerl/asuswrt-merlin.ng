#
# Until router/apps infra is ported to 33940 buildroot framework
# use this file to bring up wifi stack with minimal configuration
#

#check wifi mods/apps availability
wifi_mods_dir=/lib/modules/`uname -r`

wifi_configif() {
	ifavail=`ifconfig -a | grep ${2}`
	if [[ ! -z ${ifavail} ]] ; then
		wl -i $2 down
		wl -i $2 ap 1
		wl -i $2 ssid ${2}33940AP
		wl -i $2 up
		ifconfig $2 up
	fi
}

if [[ -e ${wifi_mods_dir}/wl.ko ]]  &&
   [[ -e ${wifi_mods_dir}/dhd.ko ]] ; then
	cd ${wifi_mods_dir};
	insmod wlcsm.ko;
	insmod wlerouter.ko;
	insmod hnd.ko;
	insmod emf.ko;
	insmod igs.ko;
	insmod wl.ko;
	instance_base=`ifconfig -a | grep wl | wc -l`
	insmod dhd.ko instance_base=${instance_base}

	wifi_configif br0 wl0
	wifi_configif br0 wl1
	wifi_configif br0 wl2
fi
