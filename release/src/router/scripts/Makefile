#
# Copyright 2014  Broadcom Corporation
#

build dynamic install: bin-install init-install rc3-install
clean:                 bin-clean   init-clean   rc3-clean

CURR_DIR  := $(shell pwd)
BUILD_DIR := $(HND_SRC)
include $(BUILD_DIR)/make.common

SCRIPTS:=
RC3:=
BIN:=
INIT:=

ifeq ($(filter-out 63138 63148 4908,${BRCM_CHIP}),)
INIT += pwr/disk.sh pwr/wifi.sh pwr/switch.sh
endif

ifeq ($(strip $(BRCM_ROOTFS_RW)),y) # true
INIT += fsys/rw-rootfs.sh
RC3  += S70rw-rootfs
endif

ifneq ($(strip $(BRCM_DRIVER_SPU)),)
ifneq ($(strip $(BRCM_DRIVER_PKTFLOW)),)
ifeq ($(strip $(BRCM_CHIP)),4908)
BIN += spu_blog_enable
endif
endif
endif

init-install: ${INIT}
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/init.d
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/rc3.d
	test -z "${INIT}" || install -Dp ${INIT} ${INSTALL_DIR}/rom/rom/etc/init.d
init-clean:
	for f in ${INIT}; do rm -f ${INSTALL_DIR}/rom/rom/etc/init.d/$$f; done

# INIT might install targets for RC3 symlinks
# RC3 names must match /^[KS][0-9][0-9]/
rc3-install:
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/init.d
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/rc3.d
	for f in ${RC3}; do ln -fs ../init.d/$${f:3}.sh ${INSTALL_DIR}/rom/rom/etc/rc3.d/$$f; done
rc3-clean:
	for f in ${RC3}; do rm -f ${INSTALL_DIR}/rom/rom/etc/rc3.d/$$f; done

bin-install: ${BIN}
	mkdir -p $(INSTALL_DIR)/bin
	test -z "${BIN}" || install -Dp ${BIN} ${INSTALL_DIR}/bin
bin-clean: ${BIN}
	for f in ${BIN}; do rm -f ${INSTALL_DIR}/bin/$$f; done

.PHONY: build dynamic install clean bin-install init-install rc3-install bin-clean init-clean rc3-clean
