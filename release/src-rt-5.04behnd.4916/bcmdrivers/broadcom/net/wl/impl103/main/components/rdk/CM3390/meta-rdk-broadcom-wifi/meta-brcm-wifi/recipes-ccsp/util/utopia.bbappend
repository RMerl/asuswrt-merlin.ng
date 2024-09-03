FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

# SWRDKB-1359: [RDKM][3390] Add new meta-rdk-broadcom-wifi layer.
SRC_URI += "file://vlan_util_brcm93390_broadcom_wifi.patch;patchdir=.."
