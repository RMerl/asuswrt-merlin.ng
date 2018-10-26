# Toploevel makefile to build hnd dongle
#
#
#
#

export ALLSRCBASE= $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)
export WLSRCBASE= main/src
export WLCFGDIR = $(ALLSRCBASE)/main/src/wl/config
export WLMAIN = main/src

export PLT = arm
export LINUX_VERSION = 4_1_0
export LINUXDIR = $(KERNEL_DIR)
export 	KBUILD_VERBOSE := 1
SUBMAKE_SETTINGS = SRCBASE=$(SRCBASE) BASEDIR=$(BASEDIR)
DONGLE_FW_PATH = $(ALLSRCBASE)/sys/src/dongle
ENVRAM_PATH = $(ALLSRCBASE)/main/components/router/envram

-include $(KERNEL_DIR)/.config
EXT_CPU_ARCH_NAME := $(subst \",,$(CONFIG_BCM_CPU_ARCH_NAME))
export EXT_CPU_ARCH_NAME
prebuilt_files = $(shell find . -name *$(EXT_CPU_ARCH_NAME) -print)

.PHONY: oldconfig version pciefd clean version_info loadscript

check_prebuilt:
	$(info check_prebuilt: EXT_CPU_ARCH_NAME=$(EXT_CPU_ARCH_NAME))
	$(foreach f,$(prebuilt_files),$(shell cp -pf $(f) $(subst -$(shell echo $(EXT_CPU_ARCH_NAME)), ,$(f))))
	if [ -d $(DONGLE_FW_PATH)/mfg ]; then \
		mkdir -p $(INSTALL_DIR)/etc/wlan/dhd; \
		cp -rf $(DONGLE_FW_PATH)/mfg $(INSTALL_DIR)/etc/wlan/dhd; \
	fi ;
	if [ -e $(ENVRAM_PATH)/envram ]; then \
		mkdir -p $(INSTALL_DIR)/usr/sbin; \
		cp -f $(ENVRAM_PATH)/envram $(INSTALL_DIR)/usr/sbin; \
		cp -f $(ENVRAM_PATH)/envrams $(INSTALL_DIR)/usr/sbin; \
	fi ;
	if [ -f $(ALLSRCBASE)/main/components/router/hnd_wl/wl_mfgtest.ko ]; then \
		mkdir -p $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra; \
		cp -f $(ALLSRCBASE)/main/components/router/hnd_wl/wl_mfgtest.ko \
			$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra; \
		cp -f $(ALLSRCBASE)/main/components/router/hnd/hnd_mfgtest.ko \
			$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra; \
	fi ;

oldconfig:
	$(MAKE) -C $(HNDDRIVER_BASE) oldconfig

version:
	$(MAKE) -C $(HNDDRIVER_BASE) version

pciefd: version
	$(MAKE) -C $(HNDDRIVER_BASE) pciefd

clean:
	$(MAKE) -C $(HNDDRIVER_BASE) pciefd-clean

version_info: check_prebuilt
	@echo $(shell echo "wl:";)
