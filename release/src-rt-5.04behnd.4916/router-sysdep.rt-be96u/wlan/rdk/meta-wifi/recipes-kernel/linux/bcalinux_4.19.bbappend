FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

TARGET_FS_INSTALL_ROOT="${S}/targets/${BRCM_SOC_PROFILE}/fs.install"

#example:
#install_file "source/source.sh" "dest"
#same as
#install_file "source/source.sh" "dest/source.sh"
#install_file "source" "dest" '*.sh'
#install_file "source" "dest" '*'
#when copy and rename
#install_file "source" "dest" "source.sh" "dest.sh"

install_file() {
    local source_dir="$1"
    local dest_dir="$2"
    local source_filename="$3"
    local dest_filename=""
    echo "souce_dir:${source_dir}"
    echo "dest_dir:${dest_dir}"

    if [[ -z $3 ]]; then
        source_filename=$(basename $source_dir)
        source_dir=$(dirname $source_dir)
    fi
    if [[ -n $4 ]]; then
        dest_filename="$4"
    fi

    if [[ -d $source_dir ]]; then
        echo "ls $source_dir/$source_filename 2>/dev/null"
        files=$(ls $source_dir/$source_filename || echo "")
        if [[ -n $files ]]; then
            dest_dir="${dest_dir%%${source_filename}}"
            if [[ ! -d $dest_dir ]]; then
                mkdir -p  $dest_dir
            fi
            cp -av $source_dir/$source_filename $dest_dir/$dest_filename
        else
            echo "could not find file:$source_filename under $source_dir"
        fi
    else
        echo "$source_dir does not exist"
    fi
}

do_install_append() {
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/dhdctl ${D}/${base_bindir}/dhdctl
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/ceventd ${D}/${base_bindir}/ceventd
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/hapd_conf ${D}/${base_bindir}/hapd_conf
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/nvram ${D}/${base_bindir}/nvram
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/wifi_api ${D}/${base_bindir}/wifi_api
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/wifi_rdk_initd ${D}/${base_bindir}/wifi_rdk_initd
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/wifihal_test ${D}/${base_bindir}/wifihal_test
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/wlaffinity ${D}/${base_bindir}/wlaffinity
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/wlctl ${D}/${base_bindir}/wlctl
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/wlssk ${D}/${base_bindir}/wlssk
        install_file ${TARGET_FS_INSTALL_ROOT}/etc/init.d/bcm-wlan-drivers.sh ${D}/etc/init.d/bcm-wlan-drivers.sh
        install_file ${TARGET_FS_INSTALL_ROOT}/etc/init.d/hndmfg.sh ${D}/etc/init.d/hndmfg.sh
        install_file ${TARGET_FS_INSTALL_ROOT}/etc/init.d/hndnvram.sh ${D}/etc/init.d/hndnvram.sh
        install_file ${TARGET_FS_INSTALL_ROOT}/etc/init.d/mdev_wl.sh ${D}/etc/init.d/mdev_wl.sh
        install_file ${TARGET_FS_INSTALL_ROOT}/etc/init.d/wifi.sh ${D}/etc/init.d/wifi.sh
        install_file ${TARGET_FS_INSTALL_ROOT}/sbin/bcm_dwds_hot_plug.sh ${D}/etc/init.d/bcm_dwds_hot_plug.sh
        install_file ${TARGET_FS_INSTALL_ROOT}/etc/wlan ${D}/etc/wlan
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libmdm2_wifi.so ${D}/${base_libdir}/libmdm2_wifi.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libmdm_cbk_wifi.so ${D}/${base_libdir}/libmdm_cbk_wifi.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libnv2hapdcfg.so ${D}/${base_libdir}/libnv2hapdcfg.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libnvram.so ${D}/${base_libdir}/libnvram.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libssk_util.so ${D}/${base_libdir}/libssk_util.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libstaged.so ${D}/${base_libdir}/libstaged.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libwlcsm.so ${D}/${base_libdir}/libwlcsm.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libwldpd.so ${D}/${base_libdir}/libwldpd.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libwldm.so ${D}/${base_libdir}/libwldm.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libwlmdm.so ${D}/${base_libdir}/libwlmdm.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libwlsysutil.so ${D}/${base_libdir}/libwlsysutil.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/systemd/system/broadcom-wifi-drivers.service ${D}/${base_libdir}/systemd/system/broadcom-wifi-drivers.service
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/systemd/system/broadcom-wifi-hndmfg.service ${D}/${base_libdir}/systemd/system/broadcom-wifi-hndmfg.service
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/systemd/system/broadcom-wifi-hndnvram.service ${D}/${base_libdir}/systemd/system/broadcom-wifi-hndnvram.service
        install_file ${TARGET_FS_INSTALL_ROOT}/sbin/wps_pbcd ${D}/${base_bindir}/wps_pbcd
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/lib/libbcm.so ${D}/${libdir}/libbcm.so
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/lib/libbcmcrypto.so ${D}/${libdir}/libbcmcrypto.so
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/lib/libbshared.so ${D}/${libdir}/libbshared.so
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/lib/libceshared.so ${D}/${libdir}/libceshared.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libpmd.so ${D}/${base_libdir}/libpmd.so
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libpcre2-8.so.0 ${D}/${base_libdir}/libpcre2-8.so.0
        install_file ${TARGET_FS_INSTALL_ROOT}/lib/libpcre2-8.so.0.10.2 ${D}/${base_libdir}/libpcre2-8.so.0.10.2
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/lib/libi5api.so ${D}/${libdir}/libi5api.so
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/lib/libieee1905.so ${D}/${libdir}/libieee1905.so
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/lib/libshared.so ${D}/${libdir}/libshared.so
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/lib/libwbdshared.so ${D}/${libdir}/libwbdshared.so
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/lib/libwpa_client.so ${D}/${libdir}/libwpa_client.so
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/WlGetDriverCfg.sh ${D}/${sbindir}/WlGetDriverCfg.sh
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/WlGetDriverStats.sh ${D}/${sbindir}/WlGetDriverStats.sh
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/acs_cli2 ${D}/${sbindir}/acs_cli2
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/acsd2 ${D}/${sbindir}/acsd2
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/appeventd ${D}/${sbindir}/appeventd
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/aspmd ${D}/${sbindir}/aspmd
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/bsd ${D}/${sbindir}/bsd
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/debug_monitor ${D}/${sbindir}/debug_monitor
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/dhd ${D}/${sbindir}/dhd
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/ecbd ${D}/${sbindir}/ecbd
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/emf ${D}/${sbindir}/emf
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/envram ${D}/${sbindir}/envram
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/envrams ${D}/${sbindir}/envrams
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/epi_ttcp ${D}/${sbindir}/epi_ttcp
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/eventd ${D}/${sbindir}/eventd
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/hnddm.sh ${D}/${sbindir}/hnddm.sh
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/hostapd ${D}/${sbindir}/hostapd
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/hostapd_cli ${D}/${sbindir}/hostapd_cli
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/i5ctl ${D}/${sbindir}/i5ctl
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/ieee1905 ${D}/${sbindir}/ieee1905
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/igs ${D}/${sbindir}/igs
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/ssd ${D}/${sbindir}/ssd
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/bin/taskset ${D}/${sbindir}/taskset
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/toad ${D}/${sbindir}/toad
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/wb_cli ${D}/${sbindir}/wb_cli
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/wbd_master ${D}/${sbindir}/wbd_master
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/wbd_slave ${D}/${sbindir}/wbd_slave
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/wl ${D}/${sbindir}/wl
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/wl_server_socket ${D}/${sbindir}/wl_server_socket
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/wlconf ${D}/${sbindir}/wlconf
        install_file ${TARGET_FS_INSTALL_ROOT}/bin/eapd ${D}/${base_bindir}/eapd
        install_file ${TARGET_FS_INSTALL_ROOT}/sbin/rc ${D}/${base_bindir}/rc
        install_file ${TARGET_FS_INSTALL_ROOT}/sbin/wifi_setup ${D}/${base_bindir}/wifi_setup
        install_file ${TARGET_FS_INSTALL_ROOT}/sbin/erase ${D}/${base_bindir}/erase
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/wpa_cli ${D}/${sbindir}/wpa_cli
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/wpa_supplicant ${D}/${sbindir}/wpa_supplicant
        install_file ${TARGET_FS_INSTALL_ROOT}/bin ${D}/${base_bindir} '*.sh'
        install_file ${TARGET_FS_INSTALL_ROOT}/sbin ${D}/${base_bindir}  '*.sh'
        #wifi_dsps.sh for enahnced deep power down
        install_file ${TARGET_FS_INSTALL_ROOT}/usr/sbin/wifi_dsps.sh  ${D}/usr/sbin
        mkdir -p ${D}/usr/local/etc
        ln -sf /etc/wlan ${D}/usr/local/etc/wlan
}

#bcalinux recipe will only do_fecth once hal-wifi-broadcom get wifi source ready(fetch and patched""
do_fetch[depends] += "hal-wifi-broadcom:do_pre_fetch"
