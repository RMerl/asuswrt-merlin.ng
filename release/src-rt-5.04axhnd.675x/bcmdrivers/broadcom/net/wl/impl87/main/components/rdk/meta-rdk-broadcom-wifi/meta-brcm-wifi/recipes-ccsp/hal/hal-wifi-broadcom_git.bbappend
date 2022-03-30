#------------------------------------------------------
# Additional for RDKM
#------------------------------------------------------

# BRCM_WIFI_* variables should be defined in brcm-wifi-*.inc file.
BRCM_WIFI_CCSP_HAL_SRC_URI ??= ""
BRCM_WIFI_DRIVER_VER ??= "1.0"

# Additional dependancy
DEPENDS +=  "broadcom-wifi"
RDEPENDS_${PN} += "broadcom-wifi"

# Additional SRC_URI for Broadcom WiFi HAL
SRC_URI = "${BRCM_WIFI_CCSP_HAL_SRC_URI}"

# PV for Broadcom WiFi HAL
PV = "${BRCM_WIFI_DRIVER_VER}"

CFLAGS_append = " -I${PKG_CONFIG_SYSROOT_DIR}/usr/include/wifi "
CFLAGS_append += " -I${PKG_CONFIG_SYSROOT_DIR}/usr/include/wifi/proto/include "

# Additional CFLAGS for Broadcom WiFi HAL
CFLAGS += "-DRDKB_WLDM"

# Additional LDFLAGS for Broadcom WiFi HAL
LDFLAGS_append = " -lnvram -lshared -lwldm -lrt"
LDFLAGS_remove = "-llattice"
LDFLAGS_remove = "-llattice_wifi"
LDFLAGS_remove = "-litc_rpc"
LDFLAGS_remove = "-lnetsnmp"
LDFLAGS_remove = "-lmoca"
LDFLAGS_remove = "-ldma_ipc"

do_wifi_postunpack() {
    # remove non wifi_hal directories
	mv ${WORKDIR}/git/main/components/rdk/ccsp/hal/wifi ${WORKDIR}
	rm -rf ${WORKDIR}/git/*
	mv ${WORKDIR}/wifi ${WORKDIR}/git
}

addtask do_wifi_postunpack after do_unpack do_copy_brcmexternalsrc before do_patch

S = "${WORKDIR}/git/wifi/source/wifi"
