
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

python() {
    import re
    rdk_branch = d.getVar('CMF_GIT_BRANCH')
    rel_year = 0
    mt = re.match(r'rdkb-(\d+).*',rdk_branch)
    if mt:
        rel_year = int(mt.group(1))

    if rdk_branch == 'rdk-next' or re.match(r'.*2024q2.*', rdk_branch): 
        d.appendVar('SRC_URI',' file://brcm-add-encr_aes_gcmp256-rdk_next.patch')
    elif rel_year < 2024:
        d.appendVar('SRC_URI',' file://brcm-add-encr_aes_gcmp256.patch')

    if rel_year == 2024:
        d.appendVar('SRC_URI',' file://0002-add-a-few-country-code.patch')
}

do_pre_patch() {
	# convert dos to unix format
	if [ -f ${S}/wifi_hal_generic.h ]; then
		sed -i 's/\r//' ${S}/wifi_hal_generic.h
	fi
}

addtask pre_patch before do_patch after do_unpack

SRC_URI += "file://0001-support-sae-ext.patch"
