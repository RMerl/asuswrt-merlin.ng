FILESEXTRAPATHS_prepend := "${WLIMPL_PATH}/main/components/rdk/ccsp/hal:"
BRCM_WIFI_DRIVER_VER ??= "1.0"

BB_STRICT_CHECKSUM = "0"
SRC_URI = "file://wifi;name=wlanhal"
PV = "${BRCM_WIFI_DRIVER_VER}"

do_configure_prepend() {
	ln -sf ${S}/../../README ${S}/README
	ln -sf ${S}/../../COPYING ${S}/COPYING
	ln -sf ${S}/../../LICENSE ${S}/LICENSE
	touch ${S}/NEWS
	touch ${S}/AUTHORS
	touch ${S}/ChangeLog
	#Due to bcalinux compiling,some .o file maybe existing already, remove it for clean build
	rm -rf ${S}/*.o
	# temporary workaround to deal with wifi_hal.h header file difference TODO
	# cp -f ${BCALINUX_ROOT}/userspace/private/apps/wlan/cmwifi/wifi_hal.h  ${TMPDIR}/sysroots/${MACHINE}/usr/include/ccsp/wifi_hal.h
}

do_pre_fetch() {
	pushd ${BCASDK_ROOT}
	if [[ -f ./patches/utility.sh ]]; then
		./patches/utility.sh ${BCASDK_ROOT} ${WLIMPL_PATH}
	fi
	popd
}

addtask pre_fetch before do_fetch
