#
# Copyright 2014  Broadcom Corporation
#

build dynamic install: bin-install init-install rc3-install scripts-install
clean:                 bin-clean   init-clean   rc3-clean   scripts-clean

CURR_DIR  := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
include $(BUILD_DIR)/make.common

# Currently handles only RC3 scripts
INIT:=
DRV_RC3:=
BIN:=
SCRIPTS:=

# Check and add rc if wl or dhd drivers are not present in bcm-base-drivers.list
WLDRV=$(strip $(shell cat $(BUILD_DIR)/targets/fs.src/etc/init.d/bcm-base-drivers.list |grep "wl\.ko\|dhd\.ko"))
ifeq ($(WLDRV),)
DRV_RC3 += S45bcm-wlan-drivers
BIN += wlaffinity
SCRIPTS += wifi.sh
endif

FDIMGPATH=$(strip $(shell echo $(BCM_EXTFDIMAGE_PATH) | sed -e 's=\/=\\\/=g'))

all: install

init-install: ${INIT}
	@echo "wlan init-install: ${INIT}"    
	@test -z "${INIT}" || install -Dp ${INIT} ${INSTALL_DIR}/etc/init.d
init-clean:
	for f in ${INIT}; do rm -f ${INSTALL_DIR}/etc/init.d/$$f; done

rc3-install: drv-rc3-install
rc3-clean: drv-rc3-clean

# DRV_RC3 names must match /^[KS][0-9][0-9]/
drv-rc3-install:
	mkdir -p ${INSTALL_DIR}/etc/init.d/
	mkdir -p ${INSTALL_DIR}/etc/rc3.d/
	@echo "wlan rc3-install: ${DRV_RC3}"
	@for s in ${DRV_RC3}; do \
	  f=$${s}.sh;  \
	  cat $${f:3} | \
	    sed -e 's/KERNELVER=_set_by_buildFS_/KERNELVER=$(LINUX_VER_STR)/'  \
	    | sed -e 's/HNDROUTER=_set_by_buildFS_/HNDROUTER=$(BUILD_BRCM_HNDROUTER)/'  \
	    | sed -e 's/CPEROUTER=_set_by_buildFS_/CPEROUTER=$(BUILD_BRCM_CPEROUTER)/'  \
	    | sed -e 's,PROD_FW_PATH=_set_by_buildFS_,PROD_FW_PATH="${FDIMGPATH}",'  \
	    | sed -e 's/WLAN_BTEST=_set_by_buildFS_/WLAN_BTEST=$(BUILD_BTEST_TOOLS)/'  \
	    | sed -e 's/CONFIG_WLDPDCTL=_set_by_buildFS_/CONFIG_WLDPDCTL=$(BUILD_BCM_WLAN_DPDCTL)/'  \
	    | sed -e 's/BRCM_CHIP=_set_by_buildFS_/BRCM_CHIP=$(BRCM_CHIP)/'  \
	    | sed -e 's/WLAN_REMOVE_INTERNAL_DEBUG=_set_by_buildFS_/WLAN_REMOVE_INTERNAL_DEBUG=$(BUILD_BCM_WLAN_REMOVE_INTERNAL_DEBUG)/'  \
	    > ${INSTALL_DIR}/etc/init.d/$${f:3} ; \
	  chmod +x ${INSTALL_DIR}/etc/init.d/$${f:3} ;  ln -fs ../init.d/$${f:3} ${INSTALL_DIR}/etc/rc3.d/$$s; \
	done
drv-rc3-clean:
	for f in ${DRV_RC3}; do rm -f ${INSTALL_DIR}/etc/rc3.d/$$f; done

bin-install: ${BIN}
	@echo "wlan bin-install: ${BIN}"    
	@test -z "${BIN}" || install -Dp ${BIN} ${INSTALL_DIR}/bin
bin-clean: ${BIN}
	for f in ${BIN}; do rm -f ${INSTALL_DIR}/bin/$$f; done

scripts-install: ${SCRIPTS}
	@echo "wlan scripts-install: ${SCRIPTS}"    
	@for s in ${SCRIPTS}; do \
	  cat $${s} | \
	    sed -e 's/CPEROUTER=_set_by_buildFS_/CPEROUTER=$(BUILD_BRCM_CPEROUTER)/'  \
	    | sed -e 's/CONFIG_WLDPDCTL=_set_by_buildFS_/CONFIG_WLDPDCTL=$(BUILD_BCM_WLAN_DPDCTL)/'  \
	    > ${INSTALL_DIR}/etc/init.d/$${s} ; \
	  chmod +x ${INSTALL_DIR}/etc/init.d/$${s} ; \
	done
scripts-clean: 
	for s in ${SCRIPTS}; do rm -f ${INSTALL_DIR}/etc/init.d/$${s}; done

.PHONY: build dynamic install clean bin-install init-install rc3-install scripts-install bin-clean init-clean rc3-clean scripts-clean
