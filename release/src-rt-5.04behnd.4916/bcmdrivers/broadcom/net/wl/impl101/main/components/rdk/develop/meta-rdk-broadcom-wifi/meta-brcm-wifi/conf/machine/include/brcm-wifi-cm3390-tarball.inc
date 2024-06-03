# Broadcom WiFi driver configuration
BRCM_WIFI_PLATFORM ?= "CM3390"

PREFERRED_PROVIDER_hal-wifi = "hal-wifi-broadcom"
MACHINE_EXTRA_RDEPENDS += "broadcom-wifi"

# Additional CFLAGS for ccsp_soc.inc
CCSP_SOC_CFLAGS += "-D_ENABLE_BAND_STEERING_"
CCSP_SOC_CFLAGS += "-D_BEACONRATE_SUPPORT"
CCSP_SOC_CFLAGS += "-D_WIFI_AX_SUPPORT_"
CCSP_SOC_CFLAGS += "-D_WIFI_CONSOLIDATED_STANDARDS_"

# WIFI package server
# By default, BRCM_WIFI_PACKAGE_SERVER is defined as "file://".
# So you'll need to copy all WiFi packages to downloads/ directory manually.
# But if you have a local file server, you can use your server.
# For example, BRCM_WIFI_PACKAGE_SERVER = "http://your_server.com/wifi_pkg/"
BRCM_WIFI_PACKAGE_SERVER ?= "file://"

BRCM_WIFI_DRIVER_BASE_DIR ?= "wl"

BRCM_WIFI_DRIVER_SRC_URI ?= "${BRCM_WIFI_PACKAGE_SERVER}wifi_src.tgz;subdir=wl;name=wifidrv"
BRCM_WIFI_AIRIQ_URI ?= "${BRCM_WIFI_PACKAGE_SERVER}wifi_airiq.tgz;subdir=wl;name=wifidrv"
BRCM_WIFI_CCSP_HAL_SRC_URI ?= "${BRCM_WIFI_PACKAGE_SERVER}wifi_src.tgz;subdir=git;name=wlanhal"
