FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

# SWRDKB-1359: [RDKM][3390] Add new meta-rdk-broadcom-wifi layer.
SRC_URI += "file://brcmwifiinitialized.path"
WIFI_DRIVER_READY_PATH_UNIT = "brcmwifiinitialized.path"

