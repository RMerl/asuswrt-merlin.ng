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
prebuilt_files = $(shell find . -name *$(BRCM_CHIP)-$(EXT_CPU_ARCH_NAME) -print)

.PHONY: oldconfig version pciefd clean version_info loadscript bcm_headers_install

check_prebuilt:
	$(info check_prebuilt: BRCM_CHIP=$(BRCM_CHIP) EXT_CPU_ARCH_NAME=$(EXT_CPU_ARCH_NAME))
	$(foreach f,$(prebuilt_files),$(shell cp -pf $(f) $(subst _saved-$(shell echo $(BRCM_CHIP)-$(EXT_CPU_ARCH_NAME)),,$(f))))
	$(info WLTEST=$(WLTEST) BUILD_HND_MFG=$(BUILD_HND_MFG) BUILD_BCM_WLAN_NO_MFGBIN=$(BUILD_BCM_WLAN_NO_MFGBIN))
	if [ -d $(DONGLE_FW_PATH)/mfg ]; then \
		mkdir -p $(INSTALL_DIR)/etc/wlan/dhd; \
		cp -rf $(DONGLE_FW_PATH)/mfg $(INSTALL_DIR)/etc/wlan/dhd; \
	fi ;
	if [ -e $(ENVRAM_PATH)/envram ]; then \
		mkdir -p $(INSTALL_DIR)/usr/sbin; \
		cp -f $(ENVRAM_PATH)/envram $(INSTALL_DIR)/usr/sbin; \
		cp -f $(ENVRAM_PATH)/envrams $(INSTALL_DIR)/usr/sbin; \
	fi ;
	if [ "$(BUILD_BCM_WLAN_NO_MFGBIN)" != "y" ] && [ -f $(ALLSRCBASE)/main/components/router/hnd_wl/wl_mfgtest.ko ]; then \
		mkdir -p $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra; \
		cp -f $(ALLSRCBASE)/main/components/router/hnd_wl/wl_mfgtest.ko \
			$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra; \
		cp -f $(ALLSRCBASE)/main/components/router/hnd/hnd_mfgtest.ko \
			$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra; \
	fi ;
	if [ "$(WLTEST)" == "1" ] || [ "$(BUILD_HND_MFG)" == "y" ]; then \
		if [ -f $(ALLSRCBASE)/main/src/wl/linux/prebuilt/wl_mfgtest.o ]; then \
			cp -f $(ALLSRCBASE)/main/src/wl/linux/prebuilt/wl_mfgtest.o \
				$(ALLSRCBASE)/main/src/wl/linux/prebuilt/wl.o; \
		fi ; \
	fi ;

oldconfig:
	$(MAKE) -C $(HNDDRIVER_BASE) oldconfig

version:
	$(MAKE) -C $(HNDDRIVER_BASE) version

pciefd: version
	$(MAKE) -C $(HNDDRIVER_BASE) pciefd

bcm_headers_install:
	@echo "impl5x/Makefile: bcm_headers_install"
	@echo "WLTEST=$(WLTEST)  BUILD_HND_MFG=$(BUILD_HND_MFG)  BUILD_BCM_WLAN_NO_MFGBIN=$(BUILD_BCM_WLAN_NO_MFGBIN)"
	if [ "$(WLTEST)" == "1" ] || [ "$(BUILD_HND_MFG)" == "y" ]; then \
		if [ -f $(ALLSRCBASE)/main/components/router/hnd_wl/wl.ko ] && [ -d $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra ]; then \
			echo "linking wl_mfgtest.ko to generated wl.ko in fs as well as copying prebuilt."; \
			rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl_mfgtest.ko; \
			cp -vf $(ALLSRCBASE)/main/components/router/hnd_wl/wl.ko \
				$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl.ko; \
			ln -sv ./wl.ko $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl_mfgtest.ko; \
			cp -vf $(ALLSRCBASE)/main/src/wl/linux/prebuilt/wl.o \
				$(ALLSRCBASE)/main/src/wl/linux/prebuilt/wl_mfgtest.o_saved-$(BRCM_CHIP)-$(EXT_CPU_ARCH_NAME); \
			cp -vf $(ALLSRCBASE)/main/components/router/hnd_wl/wl.ko \
				$(ALLSRCBASE)/main/components/router/hnd_wl/wl_mfgtest.ko_saved-$(BRCM_CHIP)-$(EXT_CPU_ARCH_NAME); \
		fi ; \
		if [ -f $(ALLSRCBASE)/main/components/router/hnd/hnd.ko ] && [ -d $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra ]; then \
			echo "linking hnd_mfgtest.ko to generated hnd.ko in fs as well as copying prebuilt."; \
			rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd_mfgtest.ko; \
			cp -vf $(ALLSRCBASE)/main/components/router/hnd/hnd.ko \
				$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd.ko; \
			ln -sv ./hnd.ko $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd_mfgtest.ko; \
			cp -vf $(ALLSRCBASE)/main/components/router/hnd/hnd.ko \
				$(ALLSRCBASE)/main/components/router/hnd/hnd_mfgtest.ko_saved-$(BRCM_CHIP)-$(EXT_CPU_ARCH_NAME); \
		fi ; \
		if ls $(ALLSRCBASE)/sys/src/dongle/bin/*/rtecdc.bin >& /dev/null; then \
			if [ -d $(ALLSRCBASE)/43684 ] && [ -d $(INSTALL_DIR)/etc/wlan/dhd/mfg ]; then \
				echo "with dongle src, copying generated rtecdc.bin to mfg/rtecdc.bin in fs as well as prebuilt."; \
				cp -vrf $(ALLSRCBASE)/sys/src/dongle/bin/* \
					$(INSTALL_DIR)/etc/wlan/dhd/mfg/; \
				cp -vrf $(ALLSRCBASE)/sys/src/dongle/bin/* \
					$(ALLSRCBASE)/sys/src/dongle/mfg/; \
			fi ; \
			if test ! -d $(ALLSRCBASE)/43684; then \
				echo "no dongle src, copying prebuilt mfgtest rtecdc.bin to fs."; \
				mkdir -p $(INSTALL_DIR)/etc/wlan/dhd; \
				cp -vrf $(ALLSRCBASE)/sys/src/dongle/mfg/* \
					$(INSTALL_DIR)/etc/wlan/dhd/; \
			fi ; \
		fi ; \
	fi ;
	if [ "$(BUILD_BCM_WLAN_NO_MFGBIN)" == "y" ]; then \
		rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl_mfgtest.ko; \
		rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd_mfgtest.ko; \
		rm -rfv $(INSTALL_DIR)/etc/wlan/dhd/mfg/; \
	fi ;

clean:
	$(MAKE) -C $(HNDDRIVER_BASE) pciefd-clean
	$(MAKE) -C $(HNDDRIVER_BASE) clean

version_info: check_prebuilt
	@echo $(shell echo "wl:";)
