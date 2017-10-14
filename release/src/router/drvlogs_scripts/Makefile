#
# Copyright 2014  Broadcom Corporation
#

build dynamic install: bin-install init-install rc3-install scripts-install
clean:                 bin-clean   init-clean   rc3-clean   scripts-clean

CURR_DIR  := $(shell pwd)
BUILD_DIR := $(HND_SRC)
include $(BUILD_DIR)/make.common

SCRIPTS:=
RC3:=
BIN:=
INIT:=

$(info BRCM_DRIVER_LOG: $(BRCM_DRIVER_LOG))

ifeq ($(strip $(BRCM_DRIVER_LOG)),y) # true
SCRIPTS += procbcmlog/bcmlog procbcmlog/spitr 
ifeq ($(filter-out 63268,${BRCM_CHIP}),)
SCRIPTS += procbcmlog/ssrw
endif
endif

init-install: ${INIT}
	@echo "init-install: ${INIT}"    
	@test -z "${INIT}" || install -Dp ${INIT} ${INSTALL_DIR}/etc/init.d
init-clean:
	for f in ${INIT}; do rm -f ${INSTALL_DIR}/etc/init.d/$$f; done

# RC3 names must match /^[KS][0-9][0-9]/
rc3-install:
	@echo "rc3-install: ${RC3}"    
	@for f in ${RC3}; do ln -fs ../init.d/$${f:3}.sh ${INSTALL_DIR}/etc/rc3.d/$$f; done
rc3-clean:
	for f in ${RC3}; do rm -f ${INSTALL_DIR}/etc/rc3.d/$$f; done

bin-install: ${BIN}
	@echo "bin-install: ${BIN}"    
	@test -z "${BIN}" || install -Dp ${BIN} ${INSTALL_DIR}/bin
bin-clean: ${BIN}
	for f in ${BIN}; do rm -f ${INSTALL_DIR}/bin/$$f; done

scripts-install: ${SCRIPTS}
	@echo "scripts-install: ${SCRIPTS}"    
	@test -z "${SCRIPTS}" || install -Dp ${SCRIPTS} ${INSTALL_DIR}/rom/rom/scripts
scripts-clean: 
	for f in ${SCRIPTS}; do rm -f ${INSTALL_DIR}/rom/rom/scripts/$$f; done

.PHONY: build dynamic install clean bin-install init-install rc3-install scripts-install bin-clean init-clean rc3-clean scripts-clean
