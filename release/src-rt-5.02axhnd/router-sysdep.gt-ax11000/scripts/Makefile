#
# Copyright 2014  Broadcom Corporation
#

build dynamic install: bin-install init-install rc3-install
clean:                 bin-clean   init-clean   rc3-clean

BUILD_DIR := $(HND_SRC)
include ${BUILD_DIR}/make.common

RC3:=
BIN:=
INIT:= pwr/pm.sh pwr/disk.sh

ifeq ($(filter-out 63138 63148 63158 4908 6858,${BRCM_CHIP}),)
INIT += pwr/switch.sh
BIN  += pinmux

pinmux:; cat $@.${BRCM_CHIP} $@.sh > $@
.PHONY: pinmux
endif

ifeq ($(strip ${BRCM_ROOTFS_RW}),y) # true
INIT += fsys/rw-rootfs.sh
RC3  += S70rw-rootfs
endif

init-install: ${INIT}
	mkdir -p ${INSTALL_DIR}/etc/init.d
	test -z "${INIT}" || install -p ${INIT} ${INSTALL_DIR}/etc/init.d
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

.PHONY: build dynamic install clean bin-install init-install rc3-install bin-clean init-clean rc3-clean
