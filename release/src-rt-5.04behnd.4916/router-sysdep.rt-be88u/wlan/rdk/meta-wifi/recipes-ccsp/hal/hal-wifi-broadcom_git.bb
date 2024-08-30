SUMMARY = "HAL for RDK CCSP components"
HOMEPAGE = "http://github.com/belvedere-yocto/hal"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://../../LICENSE;md5=175792518e4ac015ab6696d16c4f607e"

FILESEXTRAPATHS_prepend := "${WLIMPL_PATH}/main/components/rdk/ccsp/hal:"
SRC_URI = "file://wifi;name=wlanhal"

PROVIDES = "hal-wifi"
RPROVIDES_${PN} = "hal-wifi"

DEPENDS += "virtual/kernel halinterface bcalinux "

BCALINUX_ROOT =  "${STAGING_KERNEL_DIR}"

SRCREV_wifihal = "${AUTOREV}"
SRCREV_FORMAT = "wifihal"

PV = "${RDK_RELEASE}+git${SRCPV}"

S = "${WORKDIR}/wifi/source/wifi"

# Add flags to support mesh wifi if the feature is available.
CFLAGS_append = " ${@bb.utils.contains('DISTRO_FEATURES', 'meshwifi', '-DENABLE_FEATURE_MESHWIFI', '', d)}"
CFLAGS_append = " -I=${includedir}/ccsp "
CFLAGS_append = " -DBUILD_RDKWIFI -DRDK_BUILD -DBCA_CPEROUTER_RDK -DRDKB\
		 -DWLAN_BCG_CM_LINUX -DWLAN_BCG_CM_RDKB \
		 -DRDKB_WLDM -DSKY_WLMNGR_SUPPORT_ENABLED \
"

python () {
    #import os
    #if "KUDU_REL_17_10_157" in os.path.basename(d.getVar("WIFI_SOURCE_VER",True)) and  \
    # not "KUDU_REL_17_10_157_" in os.path.basename(d.getVar("WIFI_SOURCE_VER",True)) :
    #    d.setVar("CONDITIONAL_CFLAGS",' -DRDKB_LATTICELESS')
    #else:
    d.setVar("CONDITIONAL_CFLAGS",' -I${BCALINUX_ROOT}/bcmdrivers/broadcom/net/wl/${BRCM_BOARD}/main/components/proto/include')
}

CFLAGS_append = "${@d.getVar('CONDITIONAL_CFLAGS',d,1)}"

CFLAGS_append = " -I.\
		 -I./core  \
		 -I./include  \
		 -I${BCALINUX_ROOT}/bcmdrivers/opensource/include/bcm963xx \
		 -I${BCALINUX_ROOT}/bcmdrivers/broadcom/include/bcm963xx\
		 -I${BCALINUX_ROOT}/shared/opensource/include/bcm963xx \
		 -I${BCALINUX_ROOT}/shared/broadcom/include/bcm963xx \
		 -I${BCALINUX_ROOT}/bcmdrivers/broadcom/net/wl/${BRCM_BOARD}/sys/src/include\
		 -I${BCALINUX_ROOT}/bcmdrivers/broadcom/net/wl/${BRCM_BOARD}/sys/src/wl/ppr/include\
		 -I${BCALINUX_ROOT}/bcmdrivers/broadcom/net/wl/${BRCM_BOARD}/sys/src/../../main/src/router/shared \
		 -I${BCALINUX_ROOT}/bcmdrivers/broadcom/net/wl/${BRCM_BOARD}/sys/src/../components/shared \
		 -I${BCALINUX_ROOT}/bcmdrivers/broadcom/net/wl/${BRCM_BOARD}/sys/src/../components/wlioctl/include\
		 -I${BCALINUX_ROOT}/bcmdrivers/broadcom/net/wl/${BRCM_BOARD}/sys/src/../components/shared/proto\
		 -I${BCALINUX_ROOT}/bcmdrivers/broadcom/net/wl/${BRCM_BOARD}/sys/src/shared/bcmwifi/include \
		 -I${BCALINUX_ROOT}/bcmdrivers/broadcom/net/wl/${BRCM_BOARD}/sys/src/../../main/src/include\
		 -I${BCALINUX_ROOT}/userspace/private/libs/wlcsm/wifi_all/cmwifi/libs/wlcsm/include \
		 -I${BCALINUX_ROOT}/userspace/private/apps/wlan/components/apps/wldm \
"
CFLAGS_append = " -Wno-unused-variable -Wno-unused-function  -Wno-return-type -s -Os  -fomit-frame-pointer -lgcc_s -fno-strict-aliasing -fPIC -Wall -DD11AC_IOTYPES -DPPR_API"

CFLAGS_append = " -DWIFI_HAL_VERSION_3"
LDFLAGS += " -ldl -lwldm  -lwldpd  -lrt -lcms_util -lcms_dal -lcms_msg -lbcm_flashutil \
            -lbcm_boardctl -lbcm_util  -lcrypt -lcms_core -ldl -lmdm_db \
            -lcms_qdm -lnanoxml -lvlanctl -lpwrctl -lethswctl -lethctl \
            -lm -lbridgeutil -ltmctl -lbcmmcast -lwlsysutil -lmdm_cbk_wifi -lwlmdm -lstaged -ljson-c \
            -lpthread -lnvram -lm -lshared -lbcm_flashutil -lbcm_boardctl -lgen_util -lsys_util -lwlcsm \
"

LDFLAGS_append_brcm-rdpa = " -lbdmf -lrdpactl"

LDFLAGS += " ${@bb.utils.contains('BUILD_VLANCTL', 'y', " -lvlanctl ","",d)} "
LDFLAGS += " ${@bb.utils.contains('BUILD_VLANSUBIF_VLANCTL', 'y', " -lvlansubif-vlanctl","",d)} "
LDFLAGS += " ${@bb.utils.contains('BUILD_DSL', 'y', " -latmctl -lxdslctl -lmdm_cbk_wifi ","",d)} "
LDFLAGS += " ${@bb.utils.contains('BUILD_GPON', 'y', \
                             " -lgponctl -lomciutil -lomcipm_drv -lbcmipc -lvlanctl ","",d)} "
LDFLAGS += " ${@bb.utils.contains('BUILD_EPONWAN', 'y', \
                             " -leponctl ","",d)} "

inherit autotools
PACKAGE_DEBUG_SPLIT_STYLE = "debug-without-src"

