# Broadcom WiFi driver configuration
RDKB_WIFI_PROVIDER ?= "broadcom"
BRCM_WIFI_DRIVER_VER ?= "develop"

PREFERRED_PROVIDER_hal-wifi = "hal-wifi-broadcom"
MACHINE_EXTRA_RDEPENDS += "broadcom-wifi"
MACHINE_EXTRA_RDEPENDS += "broadcom-lattice-wifi"

# WIFI package server
# By default, BRCM_WIFI_PACKAGE_SERVER is defined as "file://".
# So you'll need to copy all WiFi packages to downloads/ directory manually.
# But if you have a local file server, you can use your server.
# For example,
#   BRCM_WIFI_PACKAGE_SERVER = "http://your_server.com/wifi_pkg/"
BRCM_WIFI_PACKAGE_SERVER ?= "file://"

BRCM_WIFI_DRIVER_BASE_DIR ?= "wl"
BRCM_WIFI_DRIVER_SRC_URI ?= "${BRCM_WIFI_PACKAGE_SERVER}wifi_src.tgz;subdir=${BRCM_WIFI_DRIVER_BASE_DIR};name=wifidrv"

BRCM_WIFI_RDK_SRC_URI ?= "${BRCM_WIFI_PACKAGE_SERVER}wifi_src_rdk.tgz;subdir=${BRCM_WIFI_DRIVER_BASE_DIR};name=wifirdk"

BRCM_WIFI_LATTICE_SRC_URI ?= "${BRCM_WIFI_PACKAGE_SERVER}lattice_wifi.tgz;subdir=git;name=wlanlattice"

BRCM_WIFI_CCSP_HAL_SRC_URI ?= "${BRCM_WIFI_PACKAGE_SERVER}wifi_src_rdk.tgz;subdir=git;name=wlanhal"
