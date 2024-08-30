FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append = " \
    file://checkbrcmwifisupport.service \
    file://brcmwifiinitialized.path \
    file://checkbrcmwifisupport.path \
"

SYSTEMD_SERVICE_${PN}_remove = "checkrpiwifisupport.service"
SYSTEMD_SERVICE_${PN}_remove = "rpiwifiinitialized.path"
SYSTEMD_SERVICE_${PN}_remove = "checkrpiwifisupport.path"

SYSTEMD_SERVICE_${PN} += "checkbrcmwifisupport.service"
SYSTEMD_SERVICE_${PN} += "brcmwifiinitialized.path"
SYSTEMD_SERVICE_${PN} += "checkbrcmwifisupport.path"

do_install_append_class-target () {
    rm -rf ${D}${systemd_unitdir}/system/checkbrcmwifisupport.service
    rm -rf ${D}${systemd_unitdir}/system/brcmwifiinitialized.path
    rm -rf ${D}${systemd_unitdir}/system/checkbrcmwifisupport.path
    install -D -m 0644 ${WORKDIR}/checkbrcmwifisupport.service ${D}${systemd_unitdir}/system/checkbrcmwifisupport.service
    install -D -m 0644 ${WORKDIR}/brcmwifiinitialized.path ${D}${systemd_unitdir}/system/brcmwifiinitialized.path
    install -D -m 0644 ${WORKDIR}/checkbrcmwifisupport.path ${D}${systemd_unitdir}/system/checkbrcmwifisupport.path
}
