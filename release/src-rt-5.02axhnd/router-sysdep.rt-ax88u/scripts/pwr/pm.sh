#!/bin/sh

pwrsav_exit() {
	echo "Usage: $0 [status|suspend|resume]"
	echo "    status:  show current status of power saving features"
	echo "    suspend: suspend (turn off) power saving features"
	echo "    resume:  resume (turn on) power saving features"
	exit
}

pwrsav_run() {
	echo ============================================================
	echo $*
	eval $*
	echo ============================================================
}

PWRCTL=/bin/pwrctl
ETHCTL=/bin/ethctl
PCIE_ASPM_POL=/sys/module/pcie_aspm/parameters/policy
WLCTL=/bin/wlctl
DHDCTL=/bin/dhdctl
BCM_CPUOFF_EN=/sys/module/bcm_cpuoff/parameters/enable
UBUS4_DCM_EN=/sys/module/ubus4_dcm/parameters/enable
BDMF_SH=/bin/bs

pwrsav_status() {
	echo "current status of power saving features:"

	[ -x $PWRCTL ] && pwrsav_run $PWRCTL show

	if [ -x $ETHCTL ]; then
		ip -brief link | while read -n 5 ethn rest; do
			case $ethn in
			eth[0-9] | eth[0-9][0-9])
				pwrsav_run $ETHCTL $ethn eee
				pwrsav_run $ETHCTL $ethn eee-resolution
                                ;;
			esac
		done
	fi

	[ -r $PCIE_ASPM_POL ] && pwrsav_run cat $PCIE_ASPM_POL

	if [ -x $WLCTL ]; then
		ip -brief link | while read -n 4 wln rest; do
			case $wln in
			wl[0-9] | wl[0-9][0-9])
				pwrsav_run $WLCTL -i $wln rxchain_pwrsave_enable ;;
			esac
		done
	fi

	[ -r $BCM_CPUOFF_EN ] && pwrsav_run cat $BCM_CPUOFF_EN

	[ -r $UBUS4_DCM_EN ] && pwrsav_run cat $UBUS4_DCM_EN

	[ -x $BDMF_SH ] && pwrsav_run $BDMF_SH /b/e system clock_gate

	exit 0
}

pwrsav_suspend() {
	echo "suspending (turning off) power saving features..."

	if [ -x $PWRCTL ]; then
		pwrsav_run $PWRCTL config --ethapd off
		pwrsav_run $PWRCTL config --eee off
		pwrsav_run $PWRCTL config --dgm off
		pwrsav_run $PWRCTL config --wait off
		pwrsav_run $PWRCTL config --cpuspeed 1
		pwrsav_run $PWRCTL config --sr off
	fi

	if [ -w $PCIE_ASPM_POL ]; then
		pwrsav_run "echo default > $PCIE_ASPM_POL"
	fi

	if [ -x $WLCTL ]; then
		ip -brief link | while read -n 4 wln rest; do
			case $wln in
			wl[0-9] | wl[0-9][0-9])
				pwrsav_run $WLCTL -i $wln rxchain_pwrsave_enable 0 ;;
			esac
		done
	fi

	if [ -w $BCM_CPUOFF_EN ]; then
		pwrsav_run "echo 0 > $BCM_CPUOFF_EN"
	fi

	if [ -w $UBUS4_DCM_EN ]; then
		pwrsav_run "echo 0 > $UBUS4_DCM_EN"
	fi

	[ -x $BDMF_SH ] && pwrsav_run $BDMF_SH /b/c system clock_gate=no

	echo "done."
}

pwrsav_resume() {
	echo "resuming (turning on) power saving features..."

	if [ -x $PWRCTL ]; then
		pwrsav_run $PWRCTL config --ethapd on
		pwrsav_run $PWRCTL config --eee on
		pwrsav_run $PWRCTL config --dgm on
		pwrsav_run $PWRCTL config --wait on
		pwrsav_run $PWRCTL config --cpuspeed 256
		pwrsav_run $PWRCTL config --sr on
	fi

	if [ -w $PCIE_ASPM_POL ]; then
		pwrsav_run "echo powersave > $PCIE_ASPM_POL"
	fi

	if [ -x $WLCTL ]; then
		ip -brief link | while read -n 4 wln rest; do
			case $wln in
			wl[0-9] | wl[0-9][0-9])
				pwrsav_run $WLCTL -i $wln rxchain_pwrsave_enable 1 ;;
			esac
		done
	fi

	if [ -w $BCM_CPUOFF_EN ]; then
		pwrsav_run "echo 1 > $BCM_CPUOFF_EN"
	fi

	if [ -w $UBUS4_DCM_EN ]; then
		pwrsav_run "echo 1 > $UBUS4_DCM_EN"
	fi

	[ -x $BDMF_SH ] && pwrsav_run $BDMF_SH /b/c system clock_gate=yes

	echo "done."
}

[ $# -gt 1 ] && pwrsav_exit

case $1 in 
	status)     pwrsav_status ;;
	suspend)    pwrsav_suspend ;;
	resume)     pwrsav_resume ;;
	*)          pwrsav_exit ;;
esac
