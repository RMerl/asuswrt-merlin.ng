#
# Copyright 2014  Broadcom Corporation
#

build dynamic install: bin-install init-install rc3-install systemd-install
clean:                 bin-clean   init-clean   rc3-clean    misc-clean systemd-clean

BUILD_DIR := $(HND_SRC)
include ${BUILD_DIR}/make.common
include $(SRCBASE)/.config

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),XD4PRO))
BRCM_DATA_VOLUME_SIZE=8
else
BRCM_DATA_VOLUME_SIZE=20
endif
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RP-AX58))
BRCM_DEFAULTS_VOLUME_SIZE=2
else ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),XD4PRO))
BRCM_DEFAULTS_VOLUME_SIZE=2
else
BRCM_DEFAULTS_VOLUME_SIZE=8
endif
DATA_MOUNT_INSTALL_PATH=/etc/init.d
DATA_MOUNT_INSTALL_PATH_FOR_SED=$(shell (echo $(DATA_MOUNT_INSTALL_PATH) | sed 's/\//\\\//g'))
DEFAULT_FILES_LIST=
INIT_DEFAULT_FILES_LIST=
ifeq ($(filter-out 6846 6856 6858 6878 63158,${BRCM_CHIP}),)
DEFAULT_FILES_LIST+=pmd_calibration pmd_calibration.json pmd_temp2apd
INIT_DEFAULT_FILES_LIST+=pmd_calibration.json
endif
ifeq ($(filter-out 6855 6856,${BRCM_CHIP}),)
DEFAULT_FILES_LIST+=smtc_calibration
endif
DEFAULT_FILES_LIST:=\"$(DEFAULT_FILES_LIST)\"

RC3:= S25mount-fs K95mount-fs
BIN:=
INIT:= pwr/thermal.sh pwr/disk.sh cooked/mount-fs.sh cooked/mount-data.inc
SYSTEMD_UNITS:=

ifeq ($(filter-out 63138 63148 63158 4908 6858 63178 47622,${BRCM_CHIP}),)
INIT += pwr/switch.sh
endif

ifeq ($(filter-out 47622 63178 4912,${BRCM_CHIP}),)
INIT += pwr/wol.sh
endif

ifneq ($(strip $(BRCM_DRIVER_FLEXRM)),)
ifeq ($(filter-out 68880,${BRCM_CHIP}),)
INIT += pwr/spu_pwr.sh
endif
endif


ifeq ($(strip ${BRCM_ROOTFS_RW}),y) # true
INIT += fsys/rw-rootfs.sh
RC3  += S70rw-rootfs
endif

RC3 += 	S32check-and-restore K82check-and-restore
INIT += cooked/check-and-restore.sh
RC3 += 	S33init-defaults K82init-defaults
INIT += cooked/init-defaults.sh

ifeq ($(strip ${RDK_BUILD}${BUILD_SYSTEMD}),y)
SYSTEMD_UNITS += systemd/broadcom-bdmf.service systemd/broadcom-drivers.service \
systemd/broadcom-env-setup.service systemd/broadcom-static-nodes.service \
systemd/broadcom-mountdata.service systemd/broadcom-wanconf.service \
systemd/broadcom-wdt.service 
ifneq ($(strip ${BRCM_DRIVER_WIRELESS}),)
SYSTEMD_UNITS += systemd/broadcom-wifi-drivers.service
SYSTEMD_UNITS += systemd/broadcom-wifi-hndmfg.service
SYSTEMD_UNITS += systemd/broadcom-wifi-hndnvram.service
endif
ifneq ($(strip ${BUILD_SGS}),)
SYSTEMD_UNITS += systemd/broadcom-sgs.service
endif
ifneq ($(strip ${BUILD_DPI}),)
SYSTEMD_UNITS += systemd/broadcom-dpi.service
endif
ifneq ($(strip $(BUILD_SERDES_VI)),)
SYSTEMD_UNITS += systemd/broadcom-serdes_vi.service
endif
endif

MKE2FSOPTIONS = $(if $(BRCM_BUILD_FSCRYPT),-O encrypt)

cooked/%: std/%
	mkdir -p cooked
	cat $< \
		| sed -e "s/__BRCM_DATA_VOLUME_SIZE__/$(BRCM_DATA_VOLUME_SIZE)/" \
		| sed -e "s/__BRCM_DEFAULTS_VOLUME_SIZE__/$(BRCM_DEFAULTS_VOLUME_SIZE)/" \
		| sed -e "s/__DATA_MOUNT_INSTALL_PATH__/$(DATA_MOUNT_INSTALL_PATH_FOR_SED)/" \
		| sed -e "s/__DEFAULT_FILES_LIST__/$(DEFAULT_FILES_LIST)/" \
		| sed -e "s/__MKE2FSOPTIONS__/$(MKE2FSOPTIONS)/" \
		| sed -e "s/__INIT_DEFAULT_FILES_LIST__/$(INIT_DEFAULT_FILES_LIST)/" \
		| sed -e "s/__ASUS_JFFS2_VOLUME_SIZE__/$(JFFS2_UBI_VOLUME_SIZE)/" \
		> $@ 

misc-clean:
	rm -rf cooked/

init-install: ${INIT}
	mkdir -p ${INSTALL_DIR}$(DATA_MOUNT_INSTALL_PATH)
	test -z "${INIT}" || install -p ${INIT} ${INSTALL_DIR}$(DATA_MOUNT_INSTALL_PATH)

init-clean:
	@if [ -d $(INSTALL_DIR)/etc/init.d ]; then \
		cd ${INSTALL_DIR}/etc/init.d; rm -f ${INIT}; \
	fi

# INIT might install targets for RC3 symlinks
# RC3 names must match /^[KS][0-9][0-9]/
rc3-install:
	mkdir -p ${INSTALL_DIR}/etc/rc3.d
	for f in ${RC3}; do ln -fs ../init.d/$${f:3}.sh ${INSTALL_DIR}/etc/rc3.d/$$f; done
rc3-clean:
	@if [ -d $(INSTALL_DIR)/etc/rc3.d ]; then \
		cd ${INSTALL_DIR}/etc/rc3.d; rm -f ${RC3}; \
	fi

bin-install: ${BIN}
	mkdir -p ${INSTALL_DIR}/bin
	test -z "${BIN}" || install -p ${BIN} ${INSTALL_DIR}/bin
bin-clean: ${BIN}
	@rm -f ${BIN}; \
	if [ -d $(INSTALL_DIR)/bin ]; then \
		cd ${INSTALL_DIR}/bin; rm -f ${BIN};\
	fi

systemd-install: ${SYSTEMD_UNITS}
ifneq (${SYSTEMD_UNITS},)
	cat systemd/broadcom_mountdata.sh | sed -e "s/__DATA_MOUNT_INSTALL_PATH__/$(DATA_MOUNT_INSTALL_PATH_FOR_SED)/" \
        > cooked/broadcom_mountdata.sh

	install -d ${INSTALL_DIR}/etc && cp -f systemd/broadcom_service_env.conf ${INSTALL_DIR}/etc
	install -p systemd/broadcom_env_setup.sh ${INSTALL_DIR}/etc
	install -p cooked/broadcom_mountdata.sh ${INSTALL_DIR}/etc
	install -p systemd/broadcom_wan_stop.sh ${INSTALL_DIR}/etc
	install -d ${INSTALL_DIR}/etc/modules-load.d && cp -f systemd/broadcom_network.conf ${INSTALL_DIR}/etc/modules-load.d
	install -d ${INSTALL_DIR}/lib/systemd/system && cp -f ${SYSTEMD_UNITS} ${INSTALL_DIR}/lib/systemd/system
ifneq ($(strip $(BUILD_SERDES_VI)),)
	sed -i "s/Requires=broadcom-drivers.service/Requires=broadcom-drivers.service broadcom-serdes_vi.service/g" \
		${INSTALL_DIR}/lib/systemd/system/broadcom-wanconf.service
	sed -i "s/After=broadcom-drivers.service/After=broadcom-drivers.service broadcom-serdes_vi.service/g" \
		${INSTALL_DIR}/lib/systemd/system/broadcom-wanconf.service
endif
ifeq (${RDK_BUILD},)
	install -p systemd/passwd ${INSTALL_DIR}/etc
	install -p systemd/shadow ${INSTALL_DIR}/etc
endif
endif

systemd-clean: ${SYSTEMD_UNITS}
ifneq (${SYSTEMD_UNITS},)
	rm -rf ${INSTALL_DIR}/etc/broadcom_service_env.conf
	rm -rf ${INSTALL_DIR}/etc/broadcom_env_setup.sh
	rm -rf ${INSTALL_DIR}/etc/modules-load.d
	rm -rf ${INSTALL_DIR}/lib/systemd
	rm -f cooked/broadcom_mountdata.sh
ifeq (${RDK_BUILD},)
	rm -rf ${INSTALL_DIR}/etc/passwd
	rm -rf ${INSTALL_DIR}/etc/shadow
endif
endif

.PHONY: build dynamic install clean bin-install init-install rc3-install bin-clean init-clean rc3-clean sysytemd-install systemd-clean
