#require ccsp_common_rpi.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

DEPENDS_append = " json-c"


CFLAGS_append += " -U_COSA_SIM_ -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer -fno-strict-aliasing \
           -D_DSLH_STUN_ -D_NO_PKI_KB5_SUPPORT -D_BBHM_SSE_FILE_IO -D_ANSC_USE_OPENSSL_ \
           -D_COSA_INTEL_USG_ARM_ -D_COSA_FOR_COMCAST_ -D_NO_EXECINFO_H_\
           -DBUILD_WEB -D_NO_ANSC_ZLIB_ -D_DEBUG -U_ANSC_IPV6_COMPATIBLE_ -D_COSA_BCM_ARM_ -DUSE_NOTIFY_COMPONENT \
           -DENABLE_SD_NOTIFY -UPARODUS_ENABLE -U_PLATFORM_RASPBERRYPI_ \
           -D_ENABLE_BAND_STEERING_ -DBRCM_RDKB -DRDK_BUILD -DBCA_CPEROUTER_RDK"

EXTRA_OECONF_append = " --with-ccsp-platform=bcm --with-ccsp-arch=arm"

CFLAGS_append += "-Wno-unused-variable -Wno-unused-parameter -Wno-format"

CFLAGS_append += " -DWIFI_HAL_VERSION_3 -Wno-unused-but-set-variable -Wno-maybe-uninitialized"
CFLAGS_append += " -D_2_5G_ETHERNET_SUPPORT_"
CFLAGS_append += " -DCONFIG_DFS"

LDFLAGS += " \
	-lutopiautil \
	-lprivilege \
	-ljson-c \
	   "

SRC_URI_append = " \
    file://brcm_checkwifi.sh \
    file://bridge_mode.sh \
"

do_install_append(){

     install -m 777 ${D}/usr/bin/CcspWifiSsp -t ${D}/usr/ccsp/wifi/
     #ln -sf  /data/CcspWifiSsp ${D}/usr/ccsp/wifi/CcspWifiSsp
     #install -m 755 ${S}/scripts/cosa_start_wifiagent.sh ${D}/usr/ccsp/wifi
     install -m 777 ${WORKDIR}/brcm_checkwifi.sh ${D}/usr/ccsp/wifi/
     install -m 777 ${WORKDIR}/bridge_mode.sh ${D}/usr/ccsp/wifi/
     install -d ${D}${systemd_unitdir}/system/ccspwifiagent.service.d/
}

FILES_${PN} += " \
    ${prefix}/ccsp/wifi/CcspWifiSsp \
    ${prefix}/ccsp/wifi/brcm_checkwifi.sh \
    ${prefix}/ccsp/wifi/bridge_mode.sh \
"

python() {
    import re
    rdk_branch = d.getVar('CMF_GIT_BRANCH')
    rel_year = 0
    mt = re.match(r'rdkb-(\d+).*',rdk_branch)
    if mt:
        rel_year = int(mt.group(1))

    if rdk_branch == 'rdk-next' or ( rel_year > 2022 and not re.match(r'.*2023q1.*', rdk_branch)):
        d.appendVar('SRC_URI',' file://SWRDKB-1037_ccsp_wifi_agent_rdk_next.patch')
    else:
        d.appendVar('SRC_URI',' file://SWRDKB-1037_ccsp_wifi_agent.patch')
}


SRC_URI_append += " \
            file://SWRDKB-465_wifi_restartHostApd.patch \
            file://SWRDKB-497_apshealth.patch \
            file://0001-fix-halv3.patch \
            file://0002-fix-mgmt_frame_received_callback-not-defined.patch \
            "
