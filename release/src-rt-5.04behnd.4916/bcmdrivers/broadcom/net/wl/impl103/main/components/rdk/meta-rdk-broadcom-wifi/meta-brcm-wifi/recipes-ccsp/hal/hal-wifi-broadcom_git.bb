SUMMARY = "Broadcom WiFi RDKB HAL"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://../../LICENSE;md5=175792518e4ac015ab6696d16c4f607e"

PROVIDES = "hal-wifi"
RPROVIDES_${PN} = "hal-wifi"

DEPENDS += "virtual/kernel halinterface broadcom-wifi"
RDEPENDS_${PN} += "broadcom-wifi"

# BRCM_WIFI_* variables should be defined in brcm-wifi-*.inc file.
BRCM_WIFI_CCSP_HAL_SRC_URI ??= ""
BRCM_WIFI_DRIVER_VER ??= "1.0"

# Additional SRC_URI for Broadcom WiFi HAL
SRC_URI = "${BRCM_WIFI_CCSP_HAL_SRC_URI}"
SRC_URI += "file://rdkb_hal_wifi_broadcom_configure.patch"

SRCREV = "${AUTOREV}"
PV = "${BRCM_WIFI_DRIVER_VER}"

S = "${WORKDIR}/git/wifi/source/wifi"

CFLAGS += "-DRDKB_WLDM -D_FORTIFY_SOURCE=1"
CFLAGS += "-DWIFI_HAL_VERSION_3"
CFLAGS += "-I=${includedir}/ccsp"
CFLAGS += "-I${PKG_CONFIG_SYSROOT_DIR}/usr/include/wifi"
CFLAGS += "-I${PKG_CONFIG_SYSROOT_DIR}/usr/include/wifi/proto/include"

LDFLAGS += "-lwlcsm -lpthread -lnvram -lshared -lwldm -lrt"

do_wifi_postunpack() {
    # remove non wifi_hal directories
	mv ${WORKDIR}/git/main/components/rdk/ccsp/hal/wifi ${WORKDIR}
	rm -rf ${WORKDIR}/git/*
	mv ${WORKDIR}/wifi ${WORKDIR}/git
}
addtask do_wifi_postunpack after do_unpack do_copy_brcmexternalsrc before do_patch

inherit autotools

PACKAGE_DEBUG_SPLIT_STYLE = "debug-without-src"
