FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

# 0001 webpage wifi configure failure fix
SRC_URI += "file://0001-fix-wifi-webconf.patch;patchdir=../../.."

