#
# Copyright 2014  Broadcom Corporation
#

build dynamic install: bin-install init-install rc3-install scripts-install nvram-scripts-install
clean:                 bin-clean   init-clean   rc3-clean   scripts-clean nvram-scripts-clean

CURR_DIR  := $(shell pwd)
BUILD_DIR:=$(HND_SRC)
KERNEL_NVRAM_FILE_NAME := kernel_nvram.setting
USER_NVRAM_FILE_NAME := user_nvram.setting

include $(BUILD_DIR)/make.common
include $(BUILD_DIR)/make.wlan

# Currently handles only RC3 scripts
INIT:=
DRV_RC3:=
BIN:=
SCRIPTS:=

# check if wifi source has already support edpd
ifneq (,$(wildcard ${ALLSRCBASE}/cmwifi/apps/rc/Makefile))
WLDPD_SUPPORT=$(shell grep CMWIFI_RC_LDFLAGS ${ALLSRCBASE}/cmwifi/apps/rc/Makefile|wc -l)
endif
#
# Check and add rc if wl or dhd drivers are not present in bcm-base-drivers.list
WLDRV=$(strip $(shell cat $(BUILD_DIR)/targets/fs.src/etc/init.d/bcm-base-drivers.list |grep "wl\.ko\|dhd\.ko"))
ifeq ($(WLDRV),)
DRV_RC3 += S45bcm-wlan-drivers
BIN += wlaffinity
BIN += wlinc.sh
SCRIPTS += wifi.sh
SCRIPTS += mdev_wl.sh
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
	    | sed -e 's/RDK_EDPD_SUPPORT=RDK_EDPD_SUPPORT_HOLDER/RDK_EDPD_SUPPORT=$(WLDPD_SUPPORT)/' \
	    | sed -e 's/IS_RDK_BUILD=RDK_BUILD_HOLDER/IS_RDK_BUILD=$(RDK_BUILD)/'  \
	    > ${INSTALL_DIR}/etc/init.d/$${s} ; \
	  chmod +x ${INSTALL_DIR}/etc/init.d/$${s} ; \
	done
scripts-clean: 
	for s in ${SCRIPTS}; do rm -f ${INSTALL_DIR}/rom/rom/etc/init.d/$${s}; done

nvram-scripts-install:
	mkdir -p ${INSTALL_DIR}/rom/rom/etc/init.d/
	mkdir -p ${INSTALL_DIR}/rom/rom/etc/rc3.d/
	install -m 755 ./hndnvram.sh $(INSTALL_DIR)/rom/rom/etc/init.d
	install -m 755 ./wluboot2knvram.sh $(INSTALL_DIR)/rom/rom/etc/init.d
ifneq ($(strip $(BRCM_ENGR_ONLY)),)
	install -m 755 ./wlpreconfig.sh $(INSTALL_DIR)/rom/rom/etc/init.d
endif
	install -m 755 ./hnddm.sh $(INSTALL_DIR)/usr/sbin
	sed -e s/KERNEL_NVRAM_FILE_NAME/$(KERNEL_NVRAM_FILE_NAME)/g ./hndnvram.sh > $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)_$(BUILD_BRCM_BASE_SHELL)),y_y)
# fill in the content of "user_nvram_file" in hndnvram.sh for UNFWLCFG+BASESHELL builds
	sed -i s/user_nvram_file=\"\"/user_nvram_file=\"\\/data\\/.$(USER_NVRAM_FILE_NAME)\"/g $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
endif
ifneq ($(strip $(BUILD_RDKWIFI)),)
	sed -i s/BUILD_RDKWIFI_HOLDER/1/g  $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
else
	sed -i s/BUILD_RDKWIFI_HOLDER/0/g  $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
endif
ifneq ($(strip $(WLCSM_DEBUG_TRACE)),)
	sed -i s/DEBUG_DEAMON_HOLDER/1/g  $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
else
	sed -i s/DEBUG_DEAMON_HOLDER/0/g  $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
endif
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S40hndnvram; ln -s ../init.d/hndnvram.sh S40hndnvram)
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S41wluboot2knvram; ln -s ../init.d/wluboot2knvram.sh S41wluboot2knvram)
ifneq ($(strip $(BRCM_ENGR_ONLY)),)
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S42wlpreconfig; ln -s ../init.d/wlpreconfig.sh S42wlpreconfig)
endif
	install -m 755 ./hndmfg.sh $(INSTALL_DIR)/rom/rom/etc/init.d
	sed -e s/KERNEL_NVRAM_FILE_NAME/$(KERNEL_NVRAM_FILE_NAME)/g ./hndmfg.sh > $(INSTALL_DIR)/rom/rom/etc/init.d/hndmfg.sh
	(cd $(INSTALL_DIR)/rom/rom/etc/rc3.d; rm -f S39hndmfg; ln -s ../init.d/hndmfg.sh S39hndmfg)
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/wlan
	ls -l $(INSTALL_DIR)/rom/rom/etc/wlan
	echo "--------------------------------------------"
ifeq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
	install -m 755 ../nvram/kernel_nvram.setting $(INSTALL_DIR)/rom/rom/etc/wlan
else
# give empty content as original kernel_nvram.setting
	touch $(INSTALL_DIR)/rom/rom/etc/wlan/$(KERNEL_NVRAM_FILE_NAME)
endif

ifneq ($(strip $(CONFIG_BCM96846)),)
# dhda_rnr_offloverride - Override offload support setting, bit[1:0] with each bit position corresponding to a radio
#   possible values
#   - 0 (no offload), 1 (offload dhd0), 2 (offload dhd1), 3 (offload dhd0)
	echo "dhd_rnr_offload_override=1" >> $(INSTALL_DIR)/rom/rom/etc/wlan/kernel_nvram.setting
endif
ifneq ($(strip $(BRCM_NVRAM_INCLUDE_IMAGE)),)
	mkdir -p $(INSTALL_DIR)/rom/rom/etc/wlan/nvram/
	-cp -f $(BUILD_DIR)/targets/nvram/$(BRCM_CHIP)/*.nvm $(INSTALL_DIR)/rom/rom/etc/wlan/nvram/
endif

nvram-scripts-clean:
	rm -f $(INSTALL_DIR)/rom/rom/etc/wlan/kernel_nvram.setting
	rm -f $(INSTALL_DIR)/rom/rom/etc/init.d/hndnvram.sh
	rm -f $(INSTALL_DIR)/rom/rom/etc/init.d/wluboot2knvram.sh
ifneq ($(strip $(BRCM_ENGR_ONLY)),)
	rm -f $(INSTALL_DIR)/rom/rom/etc/init.d/wlpreconfig.sh
endif
	rm -f $(INSTALL_DIR)/rom/rom/etc/init.d/hndmfg.sh
	rm -f $(INSTALL_DIR)/usr/sbin/hnddm.sh

.PHONY: build dynamic install clean bin-install init-install rc3-install scripts-install nvram-scripts-install bin-clean init-clean rc3-clean scripts-clean nvram-scripts-clean
