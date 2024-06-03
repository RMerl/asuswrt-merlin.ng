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

# srom files
SROM_MAP_PATH := $(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl1/srom
SUPPORTED_CHIPS := 4360 4360_wl1 4352 43602 43602_wl1 4365 4366 4366_wl1 4366_963138REF_BGW 4366_963138REF_BGW_wl1
SUPPORTED_CHIPS += 43664 43664_963138REF_BGW 43664_963138REF_BGW_wl1 43664_63138RREF_GFAST 43664_63138RREF_GFAST_wl1 43664_63138RREF_RNC 43664_63138RREF_RNC_wl1 43664_63138RREF_7LAN 43664_63138RREF_7LAN_wl1
SUPPORTED_CHIPS += 43217 43217_968461PRW 4352_968461PRW 43666
SUPPORTED_NVRAMVARS := bcmcmn bcm4360 bcm43602 bcm4352 bcm4365 bcm4366 bcm43664 bcm43666 bcm43217

EXT_CPU_ARCH_NAME := $(subst \",,$(CONFIG_BCM_CPU_ARCH_NAME))
export EXT_CPU_ARCH_NAME
prebuilt_files_set1 = $(shell find . -name *_saved-$(PROFILE_ARCH) -print)
prebuilt_files_set2 = $(shell find . -name *_saved-$(BRCM_CHIP)-$(PROFILE_KARCH) -print)

export BUILD_WLAN_TESTBED_AP
export BUILD_BCM_WLMLO
export BUILD_BCM_WLAUX

ifneq ($(strip $(BUILD_HND_EAP)),)
# Needed by main/components/router/Makefile.fw to build pciefd
export BCA_HND_EAP=y
endif

.PHONY: oldconfig version pciefd clean version_info loadscript modules_install

check_prebuilt:
	$(info check_prebuilt: BRCM_CHIP=$(BRCM_CHIP) PROFILE_ARCH=$(PROFILE_ARCH) PROFILE_KARCH=$(PROFILE_KARCH))
	$(foreach f,$(prebuilt_files_set1),$(shell cp -pf $(f) $(subst _saved-$(shell echo $(PROFILE_ARCH)),,$(f))))
	$(foreach f,$(prebuilt_files_set2),$(shell cp -pf $(f) $(subst _saved-$(shell echo $(BRCM_CHIP)-$(PROFILE_KARCH)),,$(f))))
	$(info WLTEST=$(WLTEST) BUILD_HND_MFG=$(BUILD_HND_MFG) BUILD_BCM_WLAN_NO_MFGBIN=$(BUILD_BCM_WLAN_NO_MFGBIN))
	if [ -d $(DONGLE_FW_PATH)/mfg ]; then \
		rm -rf $(INSTALL_DIR)/etc/wlan/ ; \
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

build_srom_map : $(addprefix $(SROM_MAP_PATH)/bcm, $(addsuffix _map.bin,$(SUPPORTED_CHIPS)))
$(SROM_MAP_PATH)/bcm%_map.bin: $(SROM_MAP_PATH)/bcm%_map.c
	rm -f $@
	if [ ! -e ${@:.bin=.c} ]; then \
		ln -sf $<  ${@:.bin=.c}; \
	fi ;
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I$(ALLSRCBASE)/main/src/include -DCONFIG_BCM9$(BRCM_CHIP) -c -o ${@:.bin=.o} ${@:.bin=.c}
	$(OBJCOPY) -j .data -O binary ${@:.bin=.o} $@
	install -d $(INSTALL_DIR)/etc/wlan
	install -m 755 $@ $(INSTALL_DIR)/etc/wlan

build_nvramvars_map : $(addprefix $(SROM_MAP_PATH)/, $(addsuffix _nvramvars.bin, $(SUPPORTED_NVRAMVARS))) $(addprefix $(SROM_MAP_PATH)/, $(addsuffix _nvramvars.bin, $(SUPPORTED_NVRAMVARS_ADDITIONAL)))

$(SROM_MAP_PATH)/%_nvramvars.bin: $(SROM_MAP_PATH)/%_nvramvars.c
	rm -f $@
	if [ -e $< ]; then \
		ln -sf $< ${@:.bin=.c}; \
		$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I$(ALLSRCBASE)/main/src/include -DCONFIG_BCM9$(BRCM_CHIP) -c -o ${@:.bin=.o} ${@:.bin=.c}; \
		$(OBJCOPY) -j .data -O binary ${@:.bin=.o} $@; \
		install -d $(INSTALL_DIR)/etc/wlan; \
		install -m 755 $@ $(INSTALL_DIR)/etc/wlan; \
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
	@echo "WLTEST=$(WLTEST)  BUILD_HND_MFG=$(BUILD_HND_MFG)"
	if [ "$(WLTEST)" == "1" ] || [ "$(BUILD_HND_MFG)" == "y" ]; then \
		if find $(ALLSRCBASE)/sys/src/dongle/bin -name rtecdc.bin >& /dev/null; then \
			if ([ -d $(ALLSRCBASE)/43684 ] || [ -d $(ALLSRCBASE)/main/src/dongle/src ]) && [ -d $(INSTALL_DIR)/etc/wlan/dhd/mfg ]; then \
				echo "with dongle src, copying generated rtecdc.bin to mfg/rtecdc.bin in fs as well as prebuilt."; \
				cp -u -vrf $(ALLSRCBASE)/sys/src/dongle/bin/* \
					$(INSTALL_DIR)/etc/wlan/dhd/mfg/; \
				cp -u -vrf $(ALLSRCBASE)/sys/src/dongle/bin/* \
					$(ALLSRCBASE)/sys/src/dongle/mfg/; \
			fi ; \
			if [ ! -d $(ALLSRCBASE)/43684 ] && [ ! -d $(ALLSRCBASE)/main/src/dongle/src ]; then \
				echo "no dongle src, copying prebuilt mfgtest rtecdc.bin to fs."; \
				mkdir -p $(INSTALL_DIR)/etc/wlan/dhd; \
				cp -u -vrf $(ALLSRCBASE)/sys/src/dongle/mfg/* \
					$(INSTALL_DIR)/etc/wlan/dhd/; \
			fi ; \
		fi ; \
	fi ;

modules_install:
	@echo "wl implx/Makefile: modules_install"
	@echo "WLTEST=$(WLTEST)  BUILD_HND_MFG=$(BUILD_HND_MFG)  BUILD_BCM_WLAN_NO_MFGBIN=$(BUILD_BCM_WLAN_NO_MFGBIN)"
	if [ "$(WLTEST)" == "1" ] || [ "$(BUILD_HND_MFG)" == "y" ]; then \
		if [ -f $(ALLSRCBASE)/main/components/router/hnd_wl/wl.ko ] && [ -d $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra ]; then \
			echo "linking wl_mfgtest.ko to generated wl.ko in fs as well as copying prebuilt."; \
			rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl_mfgtest.ko; \
			cp -u -vf $(ALLSRCBASE)/main/components/router/hnd_wl/wl.ko \
				$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl.ko; \
			ln -sv ./wl.ko $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/wl_mfgtest.ko; \
			cp -u -vf $(ALLSRCBASE)/main/src/wl/linux/prebuilt/wl.o \
				$(ALLSRCBASE)/main/src/wl/linux/prebuilt/wl_mfgtest.o_saved-$(BRCM_CHIP)-$(PROFILE_KARCH); \
			cp -u -vf $(ALLSRCBASE)/main/components/router/hnd_wl/wl.ko \
				$(ALLSRCBASE)/main/components/router/hnd_wl/wl_mfgtest.ko_saved-$(BRCM_CHIP)-$(PROFILE_KARCH); \
		fi ; \
		if [ -f $(ALLSRCBASE)/main/components/router/hnd/hnd.ko ] && [ -d $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra ]; then \
			echo "linking hnd_mfgtest.ko to generated hnd.ko in fs as well as copying prebuilt."; \
			rm -fv $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd_mfgtest.ko; \
			cp -u -vf $(ALLSRCBASE)/main/components/router/hnd/hnd.ko \
				$(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd.ko; \
			ln -sv ./hnd.ko $(INSTALL_DIR)/lib/modules/$(LINUX_VER_STR)/extra/hnd_mfgtest.ko; \
			cp -u -vf $(ALLSRCBASE)/main/components/router/hnd/hnd.ko \
				$(ALLSRCBASE)/main/components/router/hnd/hnd_mfgtest.ko_saved-$(BRCM_CHIP)-$(PROFILE_KARCH); \
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
	rm -f $(SROM_MAP_PATH)/*.bin

# srommap/nvramvars builds only for 11ac/11n chips
FW_MKFILE=$(ALLSRCBASE)/main/components/router/Makefile.fw.be_369_1808
11AC_11N_CHIPS = $(shell grep '4366' $(FW_MKFILE))
ifneq ($(11AC_11N_CHIPS),)
version_info: check_prebuilt build_srom_map build_nvramvars_map
else
version_info: check_prebuilt
endif
	@echo $(shell echo "wl:";)
