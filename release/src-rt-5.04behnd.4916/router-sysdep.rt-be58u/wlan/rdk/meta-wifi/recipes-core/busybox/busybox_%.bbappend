
FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

SRC_URI += " \
   file://xxd.cfg \
   "
SRC_URI_append_broadband = " file://xxd.cfg"
