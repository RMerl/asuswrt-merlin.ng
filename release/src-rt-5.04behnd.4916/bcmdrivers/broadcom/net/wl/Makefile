# Toplevel makefile to build hnd dongle
#
#
#
#

export ALLSRCBASE= $(BUILD_DIR)/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)
export WLSRCBASE= main/src
export WLCFGDIR = $(ALLSRCBASE)/main/src/wl/config
export WLMAIN = main/src

export LINUXDIR = $(KERNEL_DIR)
export 	KBUILD_VERBOSE := 1
SUBMAKE_SETTINGS = SRCBASE=$(SRCBASE) BASEDIR=$(BASEDIR)
DONGLE_FW_PATH = $(ALLSRCBASE)/sys/src/dongle
DONGLE_FW_INSTALL_DIR = $(INSTALL_DIR)/$(BCM_EXTFDIMAGE_PATH)
ENVRAM_PATH = $(ALLSRCBASE)/main/components/router/envram

-include $(KERNEL_DIR)/.config
-include $(PROFILE_FILE)
-include $(BUILD_DIR)/make.wlan

EXT_CPU_ARCH_NAME := $(subst \",,$(CONFIG_BCM_CPU_ARCH_NAME))
export EXT_CPU_ARCH_NAME

ifneq ($(CONFIG_BRCM_CHIP_COMPAT_NAME),"")
BRCM_HOST_CHIP := $(subst $\",,$(CONFIG_BRCM_CHIP_COMPAT_NAME))
else
BRCM_HOST_CHIP := $(BRCM_CHIP)
endif

export BUILD_WLAN_TESTBED_AP
export BUILD_BCM_WLMLO
export BUILD_BCM_WLAUX

ifneq ($(strip $(BUILD_HND_EAP)),)
# Needed by main/components/router/Makefile.fw to build pciefd
export BCA_HND_EAP=y
endif

.PHONY: oldconfig version pciefd clean version_info loadscript modules_install

empty:=
space:=$(empty) $(empty)

# prebuilt objects of profiles match the arch list to be saved/restored due to binary compatible
# format: ARCH_SAVELIST := arch1 arch2 ...
ARCH_SAVELIST := WLMLO
found_tmp=$(strip $(foreach arch, $(ARCH_SAVELIST), $(if $(findstring $(arch),$(PROFILE)),$(arch))))
found_arch := $(subst $(space),,$(strip $(found_tmp)))
$(info PROFILE=$(PROFILE) found_arch=$(found_arch))

check_arch:
	$(info check_arch: BRCM_CHIP=$(BRCM_CHIP) BRCM_HOST_CHIP=$(BRCM_HOST_CHIP) PROFILE_ARCH=$(PROFILE_ARCH) PROFILE_KARCH=$(PROFILE_KARCH) found_arch=$(found_arch))
	prebuilt_files_set1=$$(find . -name "*_saved-$(BRCM_HOST_CHIP)-$(PROFILE_KARCH)$(found_arch)" -print); \
	for f in $$prebuilt_files_set1; do \
		cp -pf "$$f" "$$(echo "$$f" | sed "s/_saved-$(BRCM_HOST_CHIP)-$(PROFILE_KARCH)$(found_arch)//")"; \
	done ; \
	if [ "$(found_arch)" == "" ]; then \
		prebuilt_files_set2=$$(find . -name "*_saved-$(BRCM_HOST_CHIP)-$(PROFILE_KARCH)" -print); \
		for f in $$prebuilt_files_set2; do \
			cp -pf "$$f" "$$(echo "$$f" | sed "s/_saved-$(BRCM_HOST_CHIP)-$(PROFILE_KARCH)//")"; \
		done ; \
	fi ; \
	prebuilt_files_set3=$$(find . -name "*_saved-$(PROFILE_ARCH)" -print); \
	for f in $$prebuilt_files_set3; do \
		cp -pf "$$f" "$$(echo "$$f" | sed "s/_saved-$(PROFILE_ARCH)//")"; \
	done ; \
	if [ "$(found_arch)" != "" ] && [ -d $(DONGLE_FW_PATH)/bin$(found_arch) ]; then \
		rm -rf $(DONGLE_FW_PATH)/bin ; \
		cp -rf $(DONGLE_FW_PATH)/bin$(found_arch) $(DONGLE_FW_PATH)/bin ; \
	fi ; \
	if [ "$(found_arch)" != "" ] && [ -d $(ALLSRCBASE)/main/build/dongle$(found_arch) ]; then \
		rm -rf $(ALLSRCBASE)/main/build/dongle ; \
		cp -rf $(ALLSRCBASE)/main/build/dongle$(found_arch) $(ALLSRCBASE)/main/build/dongle ; \
	fi ;

check_prebuilt: check_arch
	$(info check_prebuilt: WLTEST=$(WLTEST) BUILD_HND_MFG=$(BUILD_HND_MFG) BUILD_BCM_WLAN_NO_MFGBIN=$(BUILD_BCM_WLAN_NO_MFGBIN) found_arch=$(found_arch))
	if [ -d $(DONGLE_FW_PATH)/mfg$(found_arch) ]; then \
		rm -rf $(INSTALL_DIR)/etc/wlan/ ; \
		cd $(DONGLE_FW_PATH)/mfg$(found_arch) && find . -name rtecdc.bin | cpio -pdm $(INSTALL_DIR)/etc/wlan/dhd/mfg; \
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
	mkdir -p $(DONGLE_FW_INSTALL_DIR)
	-cp -u -rf $(DONGLE_FW_PATH)/bin/* $(DONGLE_FW_INSTALL_DIR)
	@echo "wl implx/Makefile: done dongle build"
	@echo "WLTEST=$(WLTEST)  BUILD_HND_MFG=$(BUILD_HND_MFG) found_arch=$(found_arch)"
	if [ "$(WLTEST)" == "1" ] || [ "$(BUILD_HND_MFG)" == "y" ]; then \
		if find $(ALLSRCBASE)/sys/src/dongle/bin$(found_arch) -name rtecdc.bin >& /dev/null; then \
			if ([ -d $(ALLSRCBASE)/43684 ] || [ -d $(ALLSRCBASE)/main/src/dongle/src ]) && [ -d $(INSTALL_DIR)/etc/wlan/dhd/mfg ]; then \
				echo "with dongle src, copying generated rtecdc.bin to mfg/rtecdc.bin in fs as well as prebuilt."; \
				cp -u -vrf $(ALLSRCBASE)/sys/src/dongle/bin$(found_arch)/* \
					$(INSTALL_DIR)/etc/wlan/dhd/mfg/; \
				cp -u -vrf $(ALLSRCBASE)/sys/src/dongle/bin$(found_arch)/* \
					$(ALLSRCBASE)/sys/src/dongle/mfg$(found_arch)/; \
			fi ; \
			if [ ! -d $(ALLSRCBASE)/43684 ] && [ ! -d $(ALLSRCBASE)/main/src/dongle/src ]; then \
				echo "no dongle src, copying prebuilt mfgtest rtecdc.bin to fs."; \
				mkdir -p $(INSTALL_DIR)/etc/wlan/dhd; \
				cp -u -vrf $(ALLSRCBASE)/sys/src/dongle/mfg$(found_arch)/* \
					$(INSTALL_DIR)/etc/wlan/dhd/; \
			fi ; \
		fi ; \
	fi ;

modules_install:
	@echo "wl implx/Makefile: modules_install"
	@echo "WLTEST=$(WLTEST)  BUILD_HND_MFG=$(BUILD_HND_MFG)  BUILD_BCM_WLAN_NO_MFGBIN=$(BUILD_BCM_WLAN_NO_MFGBIN) found_arch=$(found_arch)"
	if [ "$(WLTEST)" == "1" ] || [ "$(BUILD_HND_MFG)" == "y" ]; then \
		if [ -f $(ALLSRCBASE)/main/components/router/hnd_wl/wl.ko ] && [ -d $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra ]; then \
			echo "linking wl_mfgtest.ko to generated wl.ko in fs as well as copying prebuilt."; \
			rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl_mfgtest.ko; \
			cp -u -vf $(ALLSRCBASE)/main/components/router/hnd_wl/wl.ko \
				$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl.ko; \
			ln -sv ./wl.ko $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl_mfgtest.ko; \
			cp -u -vf $(ALLSRCBASE)/main/src/wl/linux/prebuilt/wl.o \
				$(ALLSRCBASE)/main/src/wl/linux/prebuilt/wl_mfgtest.o_saved-$(BRCM_HOST_CHIP)-$(PROFILE_KARCH)$(found_arch); \
			cp -u -vf $(ALLSRCBASE)/main/components/router/hnd_wl/wl.ko \
				$(ALLSRCBASE)/main/components/router/hnd_wl/wl_mfgtest.ko_saved-$(BRCM_HOST_CHIP)-$(PROFILE_KARCH)$(found_arch); \
		fi ; \
		if [ -f $(ALLSRCBASE)/main/components/router/hnd/hnd.ko ] && [ -d $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra ]; then \
			echo "linking hnd_mfgtest.ko to generated hnd.ko in fs as well as copying prebuilt."; \
			rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd_mfgtest.ko; \
			cp -u -vf $(ALLSRCBASE)/main/components/router/hnd/hnd.ko \
				$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd.ko; \
			ln -sv ./hnd.ko $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd_mfgtest.ko; \
			cp -u -vf $(ALLSRCBASE)/main/components/router/hnd/hnd.ko \
				$(ALLSRCBASE)/main/components/router/hnd/hnd_mfgtest.ko_saved-$(BRCM_HOST_CHIP)-$(PROFILE_KARCH)$(found_arch); \
		fi ; \
	fi ;
	if [ "$(BUILD_BCM_WLAN_NO_MFGBIN)" == "y" ]; then \
		rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl_mfgtest.ko; \
		rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd_mfgtest.ko; \
		rm -rfv $(INSTALL_DIR)/etc/wlan/dhd/mfg/; \
	fi ;

clean:
	$(MAKE) -C $(HNDDRIVER_BASE) pciefd-clean
	rm -rf $(DONGLE_FW_INSTALL_DIR)

version_info: version check_prebuilt
	@echo $(shell echo "wl:";)
