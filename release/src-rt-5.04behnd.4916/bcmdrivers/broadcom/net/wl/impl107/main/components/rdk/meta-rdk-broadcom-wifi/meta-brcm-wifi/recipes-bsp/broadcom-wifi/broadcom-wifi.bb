SUMMARY = "Broadcom Wifi Driver"
DESCRIPTION = "Wifi Driver for Broadcom Wifi chipset."
SECTION = "drivers/net"
LICENSE = "CLOSED"

inherit systemd
inherit module
inherit pythonnative

DEPENDS += "virtual/kernel broadcom-rgdriver sqlite3 libxml2 openssl bison-native libnl util-linux-native json-c rsync-native"

# BRCM_WIFI_* variables should be defined in brcm-wifi-*.inc file.
BRCM_WIFI_DRIVER_SRC_URI ??= ""
BRCM_WIFI_DRIVER_BASE_DIR ??= ""
BRCM_WIFI_DRIVER_VER ??= "1.0"

# SRC_URI configuration
# OPENBFC_SDK_SRC_URI should be defined in openbfc-*.inc configuration file.
# BRCM_WIFI_DRIVER_SRC_URI should be defined in brcm-wifi-*.inc file.
SRC_URI += "${OPENBFC_SDK_SRC_URI}"
SRC_URI += "${BRCM_WIFI_DRIVER_SRC_URI}"
SRC_URI += "${BRCM_WIFI_AIRIQ_URI}"
SRC_URI += "file://rdkb_openbfc.patch;patchdir=../${OPENBFC_SDK_BASE_DIR}"
SRC_URI += "file://wifi.service"

S = "${WORKDIR}/${BRCM_WIFI_DRIVER_BASE_DIR}"
PV = "${BRCM_WIFI_DRIVER_VER}"

# RG Apps Directories
RG_APPS_BUILD_DIR = "${WORKDIR}/${OPENBFC_SDK_BASE_DIR}/rg_apps"
TARGET_INSTALL_DIR = "${S}/rg_apps/targets/${RGPROFILE}/fs.install"

# Set to 1, if you need to include WIFI debug information
INCLUDE_WIFI_DEBUG_INFO = "0"

# Limit BRCM_MAX_JOBS to 1, if you have any parallel build problem.
# RGOPTS += "BRCM_MAX_JOBS=1"

# Options for Makefile
RGOPTS += "ARCH=${ARCH}"
RGOPTS += "BRCM_CABLE=y"
RGOPTS += "PROFILE=${RGPROFILE}"
RGOPTS += "BRCM_CHIP=${BRCM_CHIP}"
RGOPTS += "PLAT=${RGPLAT}"
RGOPTS += "CROSS_COMPILE=${TARGET_PREFIX}"
RGOPTS += "BUILD_WLMNGR2=y"
RGOPTS += "SUPPORT_NO_WIFI_CMS=y"
RGOPTS += "APBUILD=mixedap"
RGOPTS += "RDKB=y"
RGOPTS += "CMWIFI_RDKB=y"
RGOPTS += "SHELL=/bin/bash"
RGOPTS += "EXTMODDIR=${RG_APPS_BUILD_DIR}/bcmdrivers/broadcom"
RGOPTS += "BUILD_DIR=${RG_APPS_BUILD_DIR}"
RGOPTS += "RGAPPS_INSTDIR=${TARGET_INSTALL_DIR}"
RGOPTS += "LINUX_VER_STR=${KERNEL_VERSION}"
RGOPTS += "KERNEL_BUILDDIR=${STAGING_KERNEL_BUILDDIR}"
RGOPTS += "KBUILD_EXTRA_SYMBOLS=${STAGING_INCDIR}/broadcom-rgdriver/Module.symvers"
RGOPTS += "BRCM_COMMON_CFLAGS='-Os -fno-strict-aliasing -Wall -Darm -g -fPIC'"

SYSTEMD_SERVICE_${PN} = "wifi.service"
EXTRA_OEMAKE += "LINUX_CONFIG=${STAGING_KERNEL_BUILDDIR}/.config"

FILES_${PN} += "${bindir}"
FILES_${PN} += "${includedir}"
FILES_${PN} += "${libdir}"
FILES_${PN} += "${base_libdir}"
FILES_${PN} += "/usr/local/etc"
FILES_${PN} += "/usr/local/sbin"
FILES_${PN} += "/webs"
FILES_${PN} += "/etc"
FILES_${PN} += "${systemd_unitdir}/system"
FILES_${PN} += "/etc/rc6.d"
FILES_${PN}_remove = "/mnt/apps/bin"

FILES_${PN} += "${S}/cmwifi"

FILES_SOLIBSDEV = ""

# Include openbfc_sdk_rdkm.inc
include conf/distro/include/openbfc_sdk_rdkm.inc

do_unpack_prepend() {
    import os
    openbfc_dir = d.getVar('OPENBFC_SDK_BASE_DIR', True) or ''
    target_cleandirs = openbfc_dir + '/rg_apps/.config '
    target_cleandirs += openbfc_dir + '/rg_apps/kernel '
    for target_item in target_cleandirs.split():
        os.system('echo Removing ' + target_item)
        os.system('rm -rf ' + target_item)
}

do_wifi_postunpack() {
    # Copy the default RGPROFILE configuration
    if [ ! -f "${RG_APPS_BUILD_DIR}/.config" ]; then
        cp -f ${RG_APPS_BUILD_DIR}/targets/${RGPROFILE}/${RGPROFILE} ${RG_APPS_BUILD_DIR}/.config
    fi
    # Create symbolic link to Yocto staging kernel
    if [ -h "${RG_APPS_BUILD_DIR}/kernel" ]; then
        rm -rf ${RG_APPS_BUILD_DIR}/kernel
        mkdir -p ${RG_APPS_BUILD_DIR}/kernel
        ln -s ${STAGING_KERNEL_DIR} ${RG_APPS_BUILD_DIR}/kernel/linux
    fi
    # Create symbolic link to rbb_lattice
	ln -s rbb_lattice ${WORKDIR}/rg
    # Untar runner driver source package
    OPENBFC_RUNNER_DIR="${RG_APPS_BUILD_DIR}/bcmdrivers/broadcom/runner"
    if [ ! -d "${OPENBFC_RUNNER_DIR}/drivers" ] && [ -f "${OPENBFC_RUNNER_DIR}/rdp_cable.tar.gz" ]; then
        tar xzf ${OPENBFC_RUNNER_DIR}/rdp_cable.tar.gz -C ${OPENBFC_RUNNER_DIR}
    fi
    # Create the target install directories
    if [ ! -e "${TARGET_INSTALL_DIR}/bin" ]; then
        mkdir -p ${TARGET_INSTALL_DIR}/bin
    fi
    # remove wifi_hal directories
	rm -rf ${S}/main/components/rdk
}
addtask do_wifi_postunpack after do_unpack do_copy_brcmexternalsrc before do_patch

do_configure[noexec] = "1"

do_compile() {
    unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE

    if [ ${INCLUDE_WIFI_DEBUG_INFO} == 1 ]; then
        #export DEBUG=1
        export WLTEST=1
    fi

    echo "Building Wifi driver"
    make ${RGOPTS} BUILD_DIR=${RG_APPS_BUILD_DIR} RGAPPS_INSTDIR=${TARGET_INSTALL_DIR} LINUX_VER_STR=${LINUX_VER_STR} -C cmwifi wlmodule
    make ${RGOPTS} BUILD_DIR=${RG_APPS_BUILD_DIR} RGAPPS_INSTDIR=${TARGET_INSTALL_DIR} -C cmwifi wlapps_prebuilt
    make ${RGOPTS} BUILD_DIR=${RG_APPS_BUILD_DIR} RGAPPS_INSTDIR=${TARGET_INSTALL_DIR} LINUX_VER_STR=${LINUX_VER_STR} -C cmwifi wifi_tarball

    # Create wifi prebuilt binary.
    rm -rf ${WORKDIR}/wlan_objs_archive*
    rm -rf ${WORKDIR}/wifi_prebuilt_bin*
    tar xzf $(ls ${S}/cmwifi/wlan_objs_archive*.tgz) -C ${WORKDIR}
    mv $(ls -d ${WORKDIR}/wlan_objs_archive*) ${WORKDIR}/wifi_prebuilt_bin
    cp ${S}/cmwifi/libs/wlcsm/include/*.h ${WORKDIR}/wifi_prebuilt_bin/
    mkdir ${WORKDIR}/wifi_prebuilt_bin/wifi
    cp -rf ${S}/main/src/include/* ${WORKDIR}/wifi_prebuilt_bin/wifi/
    cp -rf ${S}/main/src/shared/bcmwifi/include/* ${WORKDIR}/wifi_prebuilt_bin/wifi/
    cp -rf ${S}/main/components/proto ${WORKDIR}/wifi_prebuilt_bin/wifi/
    cp -rf ${S}/main/components/apps/wldm/*.h ${WORKDIR}/wifi_prebuilt_bin/wifi/
    cp -rf ${S}/main/components/wlioctl/include/*.h ${WORKDIR}/wifi_prebuilt_bin/wifi/
    cp -rf ${S}/cmwifi/release/nvram/nvram_default*.txt ${WORKDIR}/wifi_prebuilt_bin/etc/wlan/
    tar czf ${WORKDIR}/wifi_prebuilt_bin.tgz -C ${WORKDIR} wifi_prebuilt_bin
}


do_install() {
    # Regenerate wifi_prebuilt_bin from wifi_prebuilt_bin.tgz
    mkdir ${WORKDIR}/prebuilt_tmp
    tar xzf ${WORKDIR}/wifi_prebuilt_bin.tgz -C ${WORKDIR}/prebuilt_tmp
    # Install directories
    INSTALL_DIR=${WORKDIR}/prebuilt_tmp/wifi_prebuilt_bin
    install -d ${D}/${bindir}
    install -d ${D}/${includedir}
    install -d ${D}/${libdir}
    install -d ${D}/${base_libdir}/modules/${KERNEL_VERSION}
    install -d ${D}/${base_libdir}/rdk
    install -d ${D}/usr/local/etc/wlan
    install -d ${D}/usr/local/www
    install -d ${D}/usr/local/sbin
    install -d ${D}/${systemd_unitdir}/system/
    install -d ${D}/etc/rc6.d

    # Install applications
    install -m 0755 ${INSTALL_DIR}/sbin/* ${D}/${bindir}/

    # Install Kernel modules
    install -m 0644 ${INSTALL_DIR}/lib/modules/extra/* ${D}/${base_libdir}/modules/${KERNEL_VERSION}/

    # Install libraries
    # temprorarily move /modules out of /lib to avoid the installation error
    mv ${INSTALL_DIR}/lib/modules/ ${INSTALL_DIR}/
    # temprorarily move /gpl out of /lib to avoid the installation error
    # don't install wlan driver compiled nl library
    mv ${INSTALL_DIR}/lib/gpl/ ${INSTALL_DIR}/

    install -m 0644 ${INSTALL_DIR}/lib/* ${D}/${libdir}

    # move /gpl back
    mv ${INSTALL_DIR}/gpl/ ${INSTALL_DIR}/lib/
    # move /modules back
    mv ${INSTALL_DIR}/modules/ ${INSTALL_DIR}/lib/

    # Install header files
    if [ -n "$(ls ${INSTALL_DIR}/*.h)" ]; then
        install -m 0644 ${INSTALL_DIR}/*.h ${D}/${includedir}/
    fi
    if [ -n "$(ls ${INSTALL_DIR}/wifi/*.h)" ]; then
        cd ${INSTALL_DIR} && find wifi -type f -exec install -Dm 755 "{}" "${D}/${includedir}/{}" \;
    fi

    # Install miscellaneous files
    if [ -e "${INSTALL_DIR}/webs/wlrouter" ]; then
        install -d ${D}/webs/wlrouter
        install ${INSTALL_DIR}/webs/wlrouter/* ${D}/webs/wlrouter/
    fi
    install -m 0644 ${INSTALL_DIR}/etc/wlan/bcm* ${D}/usr/local/etc/wlan/
    if [ -n "$(ls ${INSTALL_DIR}/etc/wlan/nvram_default*.txt)"  ]; then
        install -m 0644 ${INSTALL_DIR}/etc/wlan/nvram_default*.txt ${D}/usr/local/etc/wlan/
    fi
    install -m 0644 ${INSTALL_DIR}/etc/wlan/nvram* ${D}/usr/local/etc/wlan/
    install -m 0644 ${WORKDIR}/wifi.service ${D}/${systemd_unitdir}/system/
    # Install K80wifi
    install -m 0755 ${INSTALL_DIR}/sbin/K80wifi ${D}/etc/rc6.d/

    # Install dongle fw images
    cp -rpf ${INSTALL_DIR}/etc/wlan/dhd ${D}/usr/local/etc/wlan

    ln -sf /usr/bin/wifi_setup.sh ${D}/${base_libdir}/rdk/wifi_setup.sh
    ln -sf /usr/bin/wifi_util.sh ${D}/${base_libdir}/rdk/wifi_util.sh
    ln -sf /usr/bin/nvram_util.sh ${D}/${base_libdir}/rdk/nvram_util.sh
    ln -sfr ${D}/${bindir}/bsd ${D}/${bindir}/bsdcli

    rm -rf ${WORKDIR}/prebuilt_tmp
}


INHIBIT_PACKAGE_STRIP = "1"
INSANE_SKIP_${PN} = "ldflags already-stripped"
INSANE_SKIP_${PN}-dev = "ldflags already-stripped"

# Splitting the kernel modules to kernel-module-* packages is not necessary for Broadcom-WiFi.
# KERNEL_MODULE_AUTOLOAD is also not necessary.
PACKAGESPLITFUNCS_remove = "split_kernel_module_packages"
KERNEL_MODULE_AUTOLOAD = ""
