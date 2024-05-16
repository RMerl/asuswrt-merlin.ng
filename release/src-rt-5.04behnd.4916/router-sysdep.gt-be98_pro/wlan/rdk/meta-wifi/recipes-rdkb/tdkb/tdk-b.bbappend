
FILESEXTRAPATHS_prepend := "${THISDIR}:"

SRC_URI_append += " \
    file://WIFIHAL.patch \
"

python() {
    import re
    rdk_branch = d.getVar('CMF_GIT_BRANCH')
    rel_year = 0
    mt = re.match(r'rdkb-(\d+).*',rdk_branch)
    if mt:
        rel_year = int(mt.group(1))

    if rdk_branch == 'rdk-next' or rel_year > 2022 :
        d.appendVar('SRC_URI',' file://tdk_platform_utility_rdk_next.patch')
    else:
        d.appendVar('SRC_URI',' file://tdk_platform_utility.patch')
}

do_install_append () {
    # append a line to file
    sed -i '$aDFS_SUPPORT=Enabled' ${D}/etc/tdk_platform.properties
    sed -i '$aAP_AUTH_MODE_OPEN=1' ${D}/etc/tdk_platform.properties
    sed -i '$aAP_AUTH_MODE_SHARED=2' ${D}/etc/tdk_platform.properties
    sed -i '$aAP_AUTH_MODE_AUTO=4' ${D}/etc/tdk_platform.properties
    sed -i '$aDEFAULT_CHANNEL_BANDWIDTH=20MHz,80MHz' ${D}/etc/tdk_platform.properties
    sed -i '$aSSID_NUMBER_OF_ENTRIES=2' ${D}/etc/tdk_platform.properties
    sed -i '$aCHANNEL_MODES_2_G_ONLY=g:11G' ${D}/etc/tdk_platform.properties
    sed -i '$aCHANNEL_MODES_2_N_ONLY=n:11NGHT20,n:11NHT40PLUS' ${D}/etc/tdk_platform.properties
    sed -i '$aCHANNEL_MODES_2_NONE=n:11NGHT20,n:11NHT40PLUS' ${D}/etc/tdk_platform.properties
    sed -i '$aCHANNEL_MODES_5_N_ONLY=ac:11ACHT40PLUS' ${D}/etc/tdk_platform.properties
    sed -i '$aCHANNEL_MODES_5_AC_ONLY=ac:11ACHT40PLUS' ${D}/etc/tdk_platform.properties
    sed -i '$aCHANNEL_MODES_5_NONE=ac:11ACHT40PLUS' ${D}/etc/tdk_platform.properties

    # match and replace
    sed -i 's/AP_IF_NAME_2G=ath0/AP_IF_NAME_2G=wl0/g' ${D}/etc/tdk_platform.properties
    sed -i 's/AP_IF_NAME_5G=ath1/AP_IF_NAME_5G=wl1/g' ${D}/etc/tdk_platform.properties
    sed -i 's/RADIO_IF_2G=wifi0/RADIO_IF_2G=wl0/g' ${D}/etc/tdk_platform.properties
    sed -i 's/RADIO_IF_5G=wifi1/RADIO_IF_5G=wl1/g' ${D}/etc/tdk_platform.properties
}

