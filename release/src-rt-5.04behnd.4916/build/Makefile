#***********************************************************************
#
#  Copyright (c) 2004  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

# Top-level Makefile
show_vars = $(info $(foreach v,$1,$v='$(value $v)'))
$(call show_vars,MAKELEVEL MAKEFLAGS MAKECMDGOALS MAKEOVERRIDES MAKEFILE_LIST)
$(call show_vars,MY_DEFAULT_ANY_FIRST_RUN MY_MKENV_FIRST_RECURSION)

###########################################
# Start of the real part
###########################################

SUB_BUILD_DIR = $(CURDIR)/build
CREATE_INSTALL = $(SUB_BUILD_DIR)/.done_create_install
PRE_KERNELBUILD = $(SUB_BUILD_DIR)/.done_pre_kernelbuild
KERNELLINKS = $(SUB_BUILD_DIR)/.done_kernellinks
BCMDRIVERS_AUTOGEN = $(SUB_BUILD_DIR)/.done_bcmdrivers_autogen
IMAGE_GOAL_COOKIE = $(CURDIR)/.image_goal

###########################################
# This is the first target in the Makefile,
# so it is also the default target.
###########################################
# Sanity_check is run separately to ensure
# that .last_profile is created before
# any build artifact
############################################

all: sanity_check
	$(MAKE) -f build/Makefile default

default: mkenv prebuild_checks all_postcheck1 packages

nand emmc: default

ifneq ($(strip $(filter default,$(MAKECMDGOALS))),)
$(shell rm -f $(IMAGE_GOAL_COOKIE))
endif
ifneq ($(strip $(filter nand emmc,$(MAKECMDGOALS))),)
$(shell echo $(strip $(filter nand emmc,$(MAKECMDGOALS))) > $(IMAGE_GOAL_COOKIE))
endif

ifneq ("$(wildcard $(IMAGE_GOAL_COOKIE))","")
IMAGE_GOAL=$(shell cat $(IMAGE_GOAL_COOKIE))
endif

$(info IMAGE GOAL is $(IMAGE_GOAL))

all_postcheck1: profile_saved_check rdp_link\
     parallel_targets gdbserver full_buildimage

# Order-only rules
#----------------------------------------------------------------------------
# work around ld internal bug when building/linking the same obj concurrently:
#	  make[3]: Entering directory `/auto/jenkins_workspace_pre/workspace/Preflight_CommEngine_Dev_962118GW/cfe/build/broadcom/build_cferam_emmc'
#	  make[4]: Entering directory `/auto/jenkins_workspace_pre/workspace/Preflight_CommEngine_Dev_962118GW/hostTools'
#	  building lzma host tool ...
#	  make[3]: Entering directory `/auto/jenkins_workspace_pre/workspace/Preflight_CommEngine_Dev_962118GW/cfe/build/broadcom/build_cferam_nand'
#	  make[4]: Entering directory `/auto/jenkins_workspace_pre/workspace/Preflight_CommEngine_Dev_962118GW/hostTools'
#	  building lzma host tool ...
#	  /tools/oss/packages/x86_64-rhel6/binutils/default/bin/ld: BFD (GNU Binutils) 2.22 internal error, aborting at merge.c line 873 in _bfd_merged_section_offset
#
#	  /tools/oss/packages/x86_64-rhel6/binutils/default/bin/ld: Please report this bug.
#
#	  collect2: ld returned 1 exit status
#	  make[4]: *** [build_cmplzma] Error 1
#----------------------------------------------------------------------------
#build_cfe_nand : | build_cfe_emmc
#build_cfe_emmc : | build_cfe_sec_nand
#build_cfe_sec_nand : | build_cfe_sec_emmc

prebuild_checks : mkenv prebuild_atom
	$(MAKE) -f build/prebuild_checks.mk default

profile_saved_check : prebuild_checks

sanity_check : profile_saved_check

rdp_link kernelbuild : sanity_check

modbuild : kernelbuild rdp_link

gdbserver : $(CREATE_INSTALL)

full_buildimage gen_credits linux_tools : parallel_targets gdbserver


.PHONY: mkenv all_postcheck1

# These post kernel top level targets can compile concurrently
parallel_targets: dynamic_cfe uboot hnd_dongle kernelbuild prepare_linux_image modbuild userspace optee atf rtpolicy_gen_metadata hosttools_image

dynamic_cfe: hosttools_bootloader

mkenv:
ifneq ($(MY_MKENV_FIRST_RECURSION),0)	# run the recipes only once per build
	@echo "############### parallel build environment start ################";
	@echo  "brcm_max_jobs: "$(BRCM_MAX_JOBS)
	@echo  "actual_max_jobs: "$(ACTUAL_MAX_JOBS)
	@echo -n "hostname: "; hostname
	@echo -n "uname: "; uname -a
	@which nproc &> /dev/null && (echo -n "processors: "; nproc) || echo "nproc not available"
	@which vmstat &> /dev/null && vmstat -SM || echo "vmstat is not available"
	@which lscpu &> /dev/null && lscpu || echo "lscpu is not available"
	@which xargs &> /dev/null && echo "" | xargs --show-limits
	@echo "################ parallel build environment end ##################"

export MY_MKENV_FIRST_RECURSION := 0

ifeq ($(strip $(filter-out nand emmc default,$(MAKECMDGOALS))),)
IS_DEFAULT_TOP_BUILD := 1
endif #ifeq ($(strip $(MAKECMDGOALS)),)

endif #ifneq ($(MY_MKENV_FIRST_RECURSION),0)

############################################################################
#
# A lot of the stuff in the original Makefile has been moved over
# to make.common.
#
############################################################################
BUILD_DIR = $(CURDIR)
include $(BUILD_DIR)/make.common

internal_check: inside_internal_check

inside_internal_check:

.PHONY: internal_check inside_internal_check

-include build/internal.mk

############################################################################
# The mapping from outside recipes to internal targets for build core 
# components from outside separately. It is expected that building all  
# recipes is equal to building by overall internal target 'default'.
#
# The caller should call every recipe following below dependencies. 
# Especially, should call recipe_bootloader, recipe_kernel and  
# recipe_userspace with PREBUILD_DONE=y to avoid invoking prebuild targets
# again concurrently.
#
# The dependencies:
# recipe_bootloader recipe_kernel recipe_userspace: recipe_prebuild
# recipe_buildimage: recipe_bootloader recipe_kernel recipe_userspace
# 
#
# Currently, RDKB uplayer build recipes call these recipes.
############################################################################
recipe_prebuild: hosttools_common headers_install
recipe_bootloader: dynamic_cfe uboot optee atf
recipe_kernel: hnd_dongle modbuild dtbs prepare_linux_image
recipe_userspace: hosttools_image userspace rtpolicy_gen_metadata
recipe_buildimage: buildimage

############################################################################
#
# Make info for voice
#
############################################################################
ifneq ($(strip $(BRCM_VOICE_SUPPORT)),)
export BRCM_VOICE_SUPPORT
BRCM_VOICE_INCLUDE_MAKE_TARGETS=1
include $(BUILD_DIR)/make.voice
endif

ifneq ($(strip $(BRCM_BCM_PCM_VER)),)
export BRCM_BCM_PCM_VER
endif
############################################################################
#
# Make info for RDP modules
#
############################################################################

rdp_link:
ifneq ($(strip $(RDP_PROJECT)),)
	$(shell echo $(INC_RDP_FLAGS) > $(KERNEL_DIR)/rdp_flags.txt)
	$(shell echo $(INC_GENERAL_FLAGS) > $(KERNEL_DIR)/rdp_general_flags.txt)
	$(MAKE) -C $(RDPSDK_DIR) PROJECT=$(RDP_PROJECT) rdp_link
endif
ifneq ($(strip $(RDP_PROJECT2)),)
	$(MAKE) -C $(RDPSDK_DIR) PROJECT=$(RDP_PROJECT2) rdp_link
endif


rdp_clean: kernel_clean
ifneq ($(strip $(RDP_PROJECT)),)
	$(MAKE) -C $(RDPSDK_DIR) PROJECT=$(RDP_PROJECT) clean
ifneq ($(strip $(RDP_PROJECT2)),)
	$(MAKE) -C $(RDPSDK_DIR) PROJECT=$(RDP_PROJECT2) clean
endif
ifneq ($(strip $(RELEASE_BUILD)),)
	$(MAKE) -C $(RDPSDK_DIR) PROJECT=$(RDP_PROJECT) distclean
endif
endif

.PHONY: rdp_link rdp_clean

############################################################################
#
# Make info for secure OS and ATF
#
############################################################################
ifneq ("$(wildcard $(BUILD_DIR)/secureos/Makefile)","")
OPTEE_SOURCE := y
else
OPTEE_SOURCE := n
endif

ifeq ($(strip $(BCM_OPTEE)),$(OPTEE_SOURCE))
export BCM_OPTEE
optee: atf
	$(MAKE) -C $(BUILD_DIR)/secureos optee_os
optee_clean:
	$(MAKE) -C $(BUILD_DIR)/secureos clean
else
optee:
optee_clean:
endif

ifeq ($(strip $(BUILD_SECURE_MONITOR))_$(strip $(DESKTOP_LINUX)),y_)
export BCM_ARMTF=y
atf: hosttools_bootloader
	$(MAKE) -C $(BUILD_DIR)/bootloaders/armtf KERNEL_ARCH=$(KERNEL_ARCH) BRCM_CHIP=$(BRCM_CHIP)
	echo "Compressing ARM Trusted Firmware using lzma"
	$(BUILD_DIR)/hostTools/cmplzma -k -2 -lzma $(BUILD_DIR)/bootloaders/armtf/armtf.elf $(BUILD_DIR)/bootloaders/armtf/armtf.bin $(BUILD_DIR)/bootloaders/armtf/armtf.lz
atf_clean:
	$(MAKE) -C $(BUILD_DIR)/bootloaders/armtf distclean
else
atf:
atf_clean:
endif

###########################################################################
#
# dsl, kernel defines
#
############################################################################
ifeq ($(strip $(BUILD_NOR_KERNEL_LZ4)),y)
KERNEL_COMPRESSION=lz4
else
KERNEL_COMPRESSION=lzma
endif

ifeq ($(strip $(BRCM_KERNEL_KALLSYMS)),y)
KERNEL_KALLSYMS=1
endif

#Set up ADSL standard
export ADSL=$(BRCM_ADSL_STANDARD)

#Set up ADSL_PHY_MODE  {file | obj}
export ADSL_PHY_MODE=file

#Set up ADSL_SELF_TEST
export ADSL_SELF_TEST=$(BRCM_ADSL_SELF_TEST)

#WLIMPL command
ifneq ($(strip $(WLIMPL)),)
export WLIMPL

SVN_IMPL:=$(patsubst IMPL%,%,$(WLIMPL))
export SVN_IMPL
#SVNTAG command
ifneq ($(strip $(SVNTAG)),)
WL_BASE := $(BUILD_DIR)/bcmdrivers/broadcom/net/wl
SVNTAG_DIR := $(shell if [ -d $(WL_BASE)/$(SVNTAG)/src ]; then echo 1; else echo 0; fi)
ifeq ($(strip $(SVNTAG_DIR)),1)
$(shell ln -sf $(WL_BASE)/$(SVNTAG)/src $(WL_BASE)/impl$(SVN_IMPL))
else
$(error There is no directory $(WL_BASE)/$(SVNTAG)/src)
endif
endif

endif

ifneq ($(strip $(BRCM_DRIVER_WIRELESS_USBAP)),)
    WLBUS ?= "usbpci"
endif
#default WLBUS for wlan pci driver
WLBUS ?="pci"
export WLBUS

# generate rt policy info meatadata
ifeq ($(wildcard $(BUILD_DIR)/userspace/public/apps/rtpolicy),)
rtpolicy_gen_metadata:
	@echo "SKIPPING GENERATE RT POLICY INFO METADATA -- no src files!"
else
ifneq ($(strip $(SKIP_USERSPACE)),)
rtpolicy_gen_metadata:
	@echo "SKIPPING GENERATE RT POLICY INFO METADATA -- SKIP_USERSPACE!"
else
rtpolicy_gen_metadata: | userspace
	$(MAKE) -C userspace/public/apps/rtpolicy/ -f Bcmbuild.mk gen_metadata
endif
endif
.PHONY: rtpolicy_gen_metadata

############################################################################
#
# When there is a directory name with the same name as a Make target,
# make gets confused.  PHONY tells Make to ignore the directory when
# trying to make these targets.
#
############################################################################
.PHONY: unittests data-model kernelbuild

#
# create a bcm_relversion.h which has our release version number, e.g.
# 4 10 02.  This allows device drivers which support multiple releases
# with a single driver image to test for version numbers.
#
BCM_SWVERSION_DIR := $(KERNEL_DIR)/include/linux/
BCM_SWVERSION_FILE := $(BCM_SWVERSION_DIR)/bcm_swversion.h
BCM_VERSION_LEVEL := $(strip $(BRCM_VERSION))
BCM_RELEASE_LEVEL := $(strip $(BRCM_RELEASE))
BCM_RELEASE_LEVEL := $(shell echo $(BCM_RELEASE_LEVEL) | sed -e 's/^0*//')
BCM_PATCH_LEVEL := $(strip $(shell echo $(BRCM_EXTRAVERSION) | cut -c1-2))
BCM_PATCH_LEVEL := $(shell echo $(BCM_PATCH_LEVEL) | sed -e 's/^0*//')

$(BCM_SWVERSION_FILE): $(BUILD_DIR)/$(VERSION_MAKE_FILE)
ifneq ($(RELEASE_BUILD),)
	@if egrep -q '^BRCM_(VERSION|RELEASE|EXTRAVERSION)=.*[^a-zA-Z0-9]' $(VERSION_MAKE_FILE) ; then \
		echo "error ... illegal character detected within version in $(VERSION_MAKE_FILE)" ; \
		exit 1 ; \
	fi
endif
	@echo "creating bcm release version header file"
	@mkdir -p $(BCM_SWVERSION_DIR)
	@echo "/* IGNORE_BCM_KF_EXCEPTION */" > $(BCM_SWVERSION_FILE)
	@echo "/* this file is automatically generated from top level Makefile */" >> $(BCM_SWVERSION_FILE)
	@echo "#ifndef __BCM_SWVERSION_H__" >> $(BCM_SWVERSION_FILE)
	@echo "#define __BCM_SWVERSION_H__" >> $(BCM_SWVERSION_FILE)
	@echo "#define BCM_REL_VERSION $(BCM_VERSION_LEVEL)" >> $(BCM_SWVERSION_FILE)
	@echo "#define BCM_REL_RELEASE $(BCM_RELEASE_LEVEL)" >> $(BCM_SWVERSION_FILE)
	@echo "#define BCM_REL_PATCH $(BCM_PATCH_LEVEL)" >> $(BCM_SWVERSION_FILE)
	@echo "#define BCM_SW_VERSIONCODE ($(BCM_VERSION_LEVEL)*65536+$(BCM_RELEASE_LEVEL)*256+$(BCM_PATCH_LEVEL))" >> $(BCM_SWVERSION_FILE)
	@echo "#define BCM_SW_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))" >> $(BCM_SWVERSION_FILE)
	@echo "#endif" >> $(BCM_SWVERSION_FILE)

BCM_KF_TXT_FILE := $(BUILD_DIR)/kernel/BcmKernelFeatures.txt
bcm_kf_kernel_txt_file := $(wildcard $(BUILD_DIR)/kernel/BcmKernelFeatures_$(PROFILE_KERNEL_VER).txt)
ifneq ($(strip $(bcm_kf_kernel_txt_file)),)
BCM_KF_TXT_FILE := $(BUILD_DIR)/kernel/BcmKernelFeatures_$(PROFILE_KERNEL_VER).txt
endif
BCM_KF_KCONFIG_FILE := kernel/bcmkernel/Kconfig.bcm_kf.${LINUX_VER_STR}
MAKEFNOTES_PL := $(HOSTTOOLS_DIR)/makefpatch/makefnotes.pl

havefeatures := $(wildcard $(BCM_KF_TXT_FILE))

ifneq ($(strip $(havefeatures)),)
.PHONY: bcm_kf_auto
# Add support for compiling vanilla kernel if BCM_KF is unset, to better
# utilize the Coverity tool.
# Use "BCM_KF= " in the command line to trigger, e.g.,
# 	$make PROFILE=962118GW BCM_KF= kernelbuild
# Back to normal with the usual command line, e.g.,
# 	$make PROFILE=962118GW kernelbuild

$(BCM_KF_KCONFIG_FILE) : $(BCM_KF_TXT_FILE)
	perl $(MAKEFNOTES_PL) -kconfig -fl $(BCM_KF_TXT_FILE) > $(BCM_KF_KCONFIG_FILE)
endif



#
# Top level userspace target
#
prepare_userspace: sanity_check $(CREATE_INSTALL) data-model $(BCM_SWVERSION_FILE) $(KERNELLINKS) rdp_link hosttools_userspace

ifneq ($(strip $(SKIP_USERSPACE)),)
userspace:
	@echo "SKIPPING USERSPACE BUILD!"
else
userspace: prepare_userspace headers_install openwrt prepare_opensync_gw openjdk
	@echo "USERSPACE STARTED"
	$(MAKE) -C userspace
	@echo "USERSPACE ENDED"
endif

.PHONY: prepare_userspace userspace



# TODO begin: building userspace with OVS is special where userspace still depends on kernelspace.
# expect to remove the dependency in future.
ifeq ($(strip $(BRCM_OVS_SUPPORT)),y)
userspace: modbuild
endif # BRCM_OVS_SUPPORT
# TODO end



#
# OpenWrt targets. There are several flavors of OpenWrt support in BDK.
# 
# 1. BUILD_BRCM_OPENWRT: will result in an Openwrt system. The Openwrt mode is
#     divided into 2 sub-modes:
# 1.1 Native Mode: No CMS or BDK (basically BASESHELL, see NATIVEOPENWRT.arch).
# 1.2 non-Native (or Carrier Managed) mode: Combines BDK components (e.g. DSL,
#     GPON, tr69c) with OpenWrt management system. Includes Broadcom ATOM.
#
# There is also a customer addon option. (TODO: provide some explanation for it).
#
#

OPENWRT_ROOTFS_UNTAR_ARGS=

ifeq ($(strip $(BUILD_OPENWRT_NATIVE)),)
ifeq ($(strip $(BUILD_PRPL_FEEDS)),)
# Non-native (Full Openwrt Carrier Managed) build.  Exclude some native Openwrt files that are in the rootfs tarball.
OPENWRT_ROOTFS_UNTAR_ARGS=-X $(BUILD_DIR)/altsdk/openwrt/exclude_files.txt
endif
endif

prepare_openwrt: sanity_check $(CREATE_INSTALL) $(KERNELLINKS)

ifeq ($(strip $(BUILD_BRCM_OPENWRT)),y)
openwrt: prepare_openwrt modbuild
	@echo "==>>ALTSDK OPENWRT build STARTED"
	$(MAKE) -C altsdk/openwrt install_full
	@echo "==>>ALTSDK OPENWRT build ENDED"
openwrt_rootfs_install: openwrt userspace
	@echo "==>>untar openwrt fs tarball to install_dir"
	# Full Openwrt build creates a tarball which we need to transfer to the BDK rootfs (INSTALL_DIR).
	# If this is a Native Openwrt build, delete some Broadcom apps (do this elsewhere?)
ifneq ($(strip $(BUILD_OPENWRT_NATIVE)),)
	find $(INSTALL_DIR) -name hostapd\* -exec rm {} \;
	find $(INSTALL_DIR) -name wpa_\* -exec rm {} \;
	find $(INSTALL_DIR) -name iw -exec rm {} \;
	find $(INSTALL_DIR) -name brctl -exec rm {} \;
endif
	if [ -f $(BUILD_DIR)/altsdk/openwrt/$(OPENWRT_TAGVER)/bin/targets/brcmbca/bcm9$(BRCM_CHIP)/openwrt-*-rootfs.tar.gz ]; then \
		tar zxvf $(BUILD_DIR)/altsdk/openwrt/$(OPENWRT_TAGVER)/bin/targets/brcmbca/bcm9$(BRCM_CHIP)/openwrt-*-rootfs.tar.gz \
		$(OPENWRT_ROOTFS_UNTAR_ARGS) -C $(INSTALL_DIR); \
	fi
else
openwrt:
	@echo "SKIPPING ALTSDK OPENWRT BUILD (not configured)"
openwrt_rootfs_install:
	@echo "SKIPPING ALTSDK OPENWRT FS_INSTALL"
endif


ifeq ($(strip $(BUILD_BRCM_OPENWRT_CUSTOMER)),)
prebuild_atom:
	@echo "SKIPPING ALTSDK OPENWRT ATOM PREBUILD!"
else
prebuild_atom:
	@echo "==>>ALTSDK OPENWRT ATOM PREBUILD STARTED"
	$(MAKE) -C altsdk/atom -f Makefile.openwrt prebuild
	@echo "==>>ALTSDK OPENWRT ATOM PREBUILD ENDED"
endif

.PHONY: prepare_openwrt openwrt prebuild_atom


#
# Opensync Gateway
# Opensync Gateway uses a few of the apps from ATOM.  Basically, we just
# need to symlink those apps into the BDK build system and build them inside
# the BDK build system.
#
ifeq ($(strip $(BUILD_OPENSYNC_GW)),)
prepare_opensync_gw:
	@echo "SKIPPING OPENSYNC_GW BUILD!"
else
prepare_opensync_gw:
	@echo "PREPARING ATOM Apps for OPENSYNC_GW BUILD..."
	$(MAKE) -C altsdk/atom -f Makefile.bdk prepare_atom
endif

#
# Open JDK
#
ifeq ($(strip $(BUILD_OPENJDK)),y)
openjdk:
	@echo "==>>ALTSDK OPENJDK build STARTED"
	$(MAKE) -C altsdk/openjdk conditional_build
	@echo "==>>ALTSDK OPENJDK build ENDED"
else
openjdk:
	@echo "SKIPPING ALTSDK OPENJDK BUILD (not configured)"
endif

#
# libcreduction is a misleading name from the old days.   See comments at the
# top of hostTools/libcreduction/Makefile to see what it really does.
#
libcreduction: parallel_targets gdbserver openwrt_rootfs_install
ifeq ($(strip $(DESKTOP_LINUX)),)
	$(MAKE) -C hostTools/libcreduction install
else
	@echo "******************** SKIP libcreduction for DESKTOP_LINUX ********************";
endif

.PHONY : libcreduction menuconfig

menuconfig:
	@cd $(INC_KERNEL_BASE); \
	$(MAKE) -C $(HOSTTOOLS_DIR)/scripts/lxdialog HOSTCC=gcc && \
	$(CONFIG_SHELL) $(HOSTTOOLS_DIR)/scripts/Menuconfig $(TARGETS_DIR)/config.in $(PROFILE)


#
# the userspace apps and libs make their own directories before
# they install, so they don't depend on this target to make the
# directory for them anymore.
#
$(CREATE_INSTALL):
		mkdir -p $(PROFILE_DIR)/fs.install/etc
		mkdir -p $(INSTALL_DIR)/bin
		mkdir -p $(INSTALL_DIR)/lib
		mkdir -p $(INSTALL_DIR)/etc/snmp
		mkdir -p $(INSTALL_DIR)/etc/iproute2
		rm -rf $(INSTALL_DIR)/opt
		mkdir -p $(INSTALL_DIR)/opt/bin
		mkdir -p $(INSTALL_DIR)/opt/modules
		mkdir -p $(INSTALL_DIR)/opt/scripts
		touch $@

$(KERNELLINKS): $(KERNEL_INCLUDE_LINK) $(KERNEL_ARM_INCLUDE_LINK)
	@touch $@

$(KERNEL_INCLUDE_LINK): $(PRE_KERNELBUILD)
	ln -s -f -T $(KERNEL_DIR)/$(INC_DIR) $(KERNEL_INCLUDE_LINK)
	@touch $@

$(KERNEL_ARM_INCLUDE_LINK): $(PRE_KERNELBUILD)
	ln -s -f -T $(KERNEL_DIR)/arch/arm/$(INC_DIR) $(KERNEL_ARM_INCLUDE_LINK)
	@touch $@

.PHONY: clean_bcmdrivers_autogen


BCMD_AG_MAKEFILE:=Makefile.autogen
BCMD_AG_KCONFIG:=Kconfig.autogen
BCMD_AG_MAKEFILE_TMP:=$(BCMD_AG_MAKEFILE).tmp
BCMD_AG_KCONFIG_TMP:=$(BCMD_AG_KCONFIG).tmp

$(BCMDRIVERS_AUTOGEN): kernel/bcmkernel/Kconfig.bcm
	@cd $(BRCMDRIVERS_DIR); echo -e "\n# Automatically generated file -- do not modify manually\n\n" > $(BCMD_AG_KCONFIG_TMP)
	@cd $(BRCMDRIVERS_DIR); echo -e "\n# Automatically generated file -- do not modify manually\n\n" > $(BCMD_AG_MAKEFILE_TMP)
	@cd $(BRCMDRIVERS_DIR); echo -e "\n\$$(info READING AG MAKEFILE)\n\n" >> $(BCMD_AG_MAKEFILE_TMP)
	@alldrivers=""; \
	 cd $(BRCMDRIVERS_DIR); \
	  for autodetect in $$(find * -type f -name autodetect); do \
		dir=$${autodetect%/*}; \
		driver=$$(grep -i "^DRIVER\|FEATURE:" $$autodetect | awk -F ': *' '{ print $$2 }'); \
		[ $$driver ] || driver=$${dir##*/}; \
		[ $$(echo $$driver | wc -w) -ne 1 ] && echo "Error parsing $$autodetect" >2 && exit 1; \
		echo "Processing $$driver ($$dir)"; \
		DRIVER=$$(echo "$${driver}" | tr '[:lower:]' '[:upper:]'); \
		echo "\$$(eval \$$(call LN_RULE_AG, CONFIG_BCM_$${DRIVER}, $$dir, \$$(LN_NAME)))" >> $(BCMD_AG_MAKEFILE_TMP); \
		if [ -e $$dir/Kconfig.autodetect ]; then \
			echo "menu \"$${DRIVER}\"" >> $(BCMD_AG_KCONFIG_TMP);\
			echo "source \"../../bcmdrivers/$$dir/Kconfig.autodetect\"" >> $(BCMD_AG_KCONFIG_TMP); \
			echo "endmenu " >> $(BCMD_AG_KCONFIG_TMP); \
			echo "" >> $(BCMD_AG_KCONFIG_TMP);\
		fi; \
		true; \
	 done; \
	 duplicates=$$(echo $$alldrivers | tr " " "\n" | sort | uniq -d | tr "\n" " "); echo $$duplicates; \
	 [ $V ] && echo "alldrivers: $$alldrivers" && echo "duplicates: $$duplicates" || true; \
	 if [ $$duplicates ]; then \
		echo "ERROR: duplicate drivers found in autodetect -- $$duplicates" >&2; \
		exit 1; \
	 fi
	@# only update the $(BCMD_AG_KCONFIG) and makefile.autogen files if they haven't changed (to prevent rebuilding):
	@cd $(BRCMDRIVERS_DIR); [ -e $(BCMD_AG_MAKEFILE) ] && cmp -s $(BCMD_AG_MAKEFILE) $(BCMD_AG_MAKEFILE_TMP) || mv $(BCMD_AG_MAKEFILE_TMP) $(BCMD_AG_MAKEFILE)
	@cd $(BRCMDRIVERS_DIR);[ -e $(BCMD_AG_KCONFIG) ] && cmp -s $(BCMD_AG_KCONFIG) $(BCMD_AG_KCONFIG_TMP) || mv $(BCMD_AG_KCONFIG_TMP) $(BCMD_AG_KCONFIG)
	@cd $(BRCMDRIVERS_DIR); rm -f $(BCMD_AG_MAKEFILE_TMP) $(BCMD_AG_KCONFIG_TMP)
	@touch $@


clean_bcmdrivers_autogen: kernel_clean
	rm -f $(BRCMDRIVERS_DIR)/$(BCMD_AG_MAKEFILE_TMP) $(BRCMDRIVERS_DIR)/$(BCMD_AG_KCONFIG_TMP) $(BRCMDRIVERS_DIR)/$(BCMD_AG_MAKEFILE) $(BRCMDRIVERS_DIR)/$(BCMD_AG_KCONFIG)
	rm -f $(BCMDRIVERS_AUTOGEN)

$(PRE_KERNELBUILD): $(CREATE_INSTALL)

ifdef BCM_KF
$(PRE_KERNELBUILD): $(BCM_SWVERSION_FILE) $(BCM_KF_KCONFIG_FILE) $(BCMDRIVERS_AUTOGEN)
else
$(PRE_KERNELBUILD): $(BCM_KF_KCONFIG_FILE)
endif
	TOOLCHAIN_TOP= $(MAKE) -f build/pre_kernelbuild.mk
	TOOLCHAIN_TOP= $(MAKE) -f build/Bcmkernel.mk prepare_bcm_driver
	@touch $@

kernelbuild: headers_install \
		$(if $(BCM_KF),rdp_link) \
		$(if $(BCM_KF),$(if $(BUILD_BRCM_CPEROUTER),version_info))
	TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk

# For old wlan release, kernelbuild and userspace still depends on hnd_dongle
# This can be removed if no needs to support these releases in future
ifneq (,$(filter $(BCM_WLIMPL),65 69 83))
kernelbuild: hnd_dongle
userspace: hnd_dongle
endif


#
# Targets related to installing kernel, bcmdriver, and shared headers to an external location.
# Install kernel headers of both archs (aarch64 and arm)
#
kernel_headers_install: $(PRE_KERNELBUILD)
	CURRENT_KARCH=aarch64 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk headers_install INSTALL_HDR_PATH=$(INC_KERNELAPI_PATH)/aarch64 KERN_TARGET=
	CURRENT_KARCH=arm TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk headers_install INSTALL_HDR_PATH=$(INC_KERNELAPI_PATH)/arm KERN_TARGET=

bcmkernel_headers_install: kernel_headers_install
	CURRENT_KARCH=aarch64 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk bcmkernel_headers_install INSTALL_HDR_PATH=$(INC_KERNELAPI_PATH)/aarch64 KERN_TARGET=
	CURRENT_KARCH=arm TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk bcmkernel_headers_install INSTALL_HDR_PATH=$(INC_KERNELAPI_PATH)/arm KERN_TARGET=

bcm_headers_install: $(PRE_KERNELBUILD)
	CURRENT_KARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -f build/Bcmkernel.mk bcm_headers_install KERN_TARGET=

shared_headers_install:
	$(MAKE) -C $(BUILD_DIR)/shared shared_headers_install

ifneq ($(strip $(PREBUILD_DONE)),y)
headers_install: kernel_headers_install bcmkernel_headers_install bcm_headers_install shared_headers_install $(KERNELLINKS)
else
headers_install:
endif

.PHONY: headers_install


linux_tools: linux_tools_perf

ifneq ($(strip $(BUILD_LINUX_PERF)),)
# If BRCM_GLIBC is not defined, tell perf LIBC_SUPPORT is there
# since perf cannot auto-detect and will try to look for glibc
# version header, which doesn't exist in MUSL.
ifeq ($(strip $(BRCM_GLIBC)),)
PERF_LIBC_SUPPORT := LIBC_SUPPORT=1
endif

# Enable BCMKF overrides
ifdef BCM_KF
PERF_EXTRA_CFLAGS := -DCONFIG_BCM_KF_MISC_BACKPORTS
endif

# Map USER_ARCH to proper arch for userspace build of a kernel source app
ifeq ($(strip $(USER_ARCH)),aarch64)
PERF_ARCH=$(KARCH)
else
PERF_ARCH=$(USER_ARCH)
endif

linux_tools_perf: $(PRE_KERNELBUILD) $(CREATE_INSTALL)
	$(MAKE) ARCH=$(PERF_ARCH) -C $(KERNEL_DIR)/tools/perf WERROR=0 EXTRA_CFLAGS="-fcommon $(PERF_EXTRA_CFLAGS)" $(PERF_LIBC_SUPPORT)
	install -m 755 $(KERNEL_DIR)/tools/perf/perf $(INSTALL_DIR)/bin/
else
linux_tools_perf:
endif


kernel_config_test: $(PRE_KERNELBUILD)
	@echo
	@echo "Building $(DIR)/config_$(PROFILE)";
	-@mkdir $(DIR) 2> /dev/null || true
	sort $(KERNEL_DIR)/.config | grep -v "^\#.*$$" | grep -v "^[[:space:]]*$$" > $(DIR)/config_$(PROFILE)
	@echo "  ... done building $(DIR)/config_$(PROFILE)";

.PHONY: kernel_config_test

ifneq ($(findstring $(strip $(KERNEL_ARCH)),aarch64 arm armsfp armhfp),)
.PHONY:dtbs justdtbs

justdtbs: profile_changed_check
dtbs: modbuild

justdtbs dtbs: headers_install
	@echo "Build dts for chip $(BRCM_CHIP)..."
	@echo "$(CURRENT_KARCH) $(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk boot=$(DTS_DIR)  dtbs"
ifneq ($(findstring 4.19, $(strip $(LINUX_VER_STR))),)
	TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk boot=$(DTS_DIR) dtbs
else
	TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk dtstree=$(DTS_DIR)/dts  dtbs
endif

DTBS := dtbs

.PHONY:dtbs_clean
dtbs_clean:
	@echo "Clean dts for chip $(BRCM_CHIP)... "
	TOOLCHAIN_TOP= $(MAKE) -C $(DTS_DIR)/dts/$(BRCM_CHIP) dtbs_clean
DTBS_CLEAN := dtbs_clean
else
DTBS :=
DTBS_CLEAN :=
endif


kernel: sanity_check $(CREATE_INSTALL) kernelbuild $(HOSTTOOLS_SET) full_buildimage

shared_link:
	if [ ! -L '$(SRCBASE)/router/hnd_shared' ]; then \
		ln -sf $(SRCBASE)/shared  $(SRCBASE)/router/hnd_shared; \
	fi
	if [ ! -L '$(SRCBASE)/router-sysdep/hnd_shared' ]; then \
		ln -sf $(SRCBASE)/shared  $(SRCBASE)/router-sysdep/hnd_shared; \
	fi

modbuild: shared_link kernelbuild
	rm -rf $(HND_SRC)/targets/$(PROFILE)/modules
	@echo "******************** Starting modbuild ********************";
ifneq ($(strip $(BRCM_DRIVER_WIRELESS)),)
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl modules_install
endif
	TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk modules INSTALL_MOD_PATH=$(PROFILE_DIR)/modules
	TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk modules_install INSTALL_MOD_PATH=$(PROFILE_DIR)/modules 
	@echo "******************** DONE modbuild ********************";

adslmodbuild:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_ADSLDRV_PATH) modules
adslmodbuildclean:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_ADSLDRV_PATH) clean

spumodbuild:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_SPUDRV_PATH) modules
spumodbuildclean:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_SPUDRV_PATH) clean

pwrmngtmodbuild:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_PWRMNGTDRV_PATH) modules
pwrmngtmodclean:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_PWRMNGTDRV_PATH) clean

enetmodbuild:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_ENETDRV_PATH) modules
enetmodclean:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_ENETDRV_PATH) clean

.PHONY: modbuild adslmodbuild spumodbuild pwrmngtmodbuild enetmodbuild modules eponmodbuild gponmodbuild adslmodule

eponmodbuild:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_EPONDRV_PATH) modules
eponmodclean:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_EPONDRV_PATH) clean

gponmodbuild:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_GPON_PATH) modules
gponmodclean:
	 TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_GPON_PATH) clean

modules: sanity_check $(CREATE_INSTALL) modbuild $(HOSTTOOLS_SET) full_buildimage

adslmodule: adslmodbuild
adslmoduleclean: adslmodbuildclean

spumodule: spumodbuild
spumoduleclean: spumodbuildclean

pwrmngtmodule: pwrmngtmodbuild
pwrmngtmoduleclean: pwrmngtmodclean

CMS2BBF_APP := cms2bbf
CMS2BBF_DIR := $(HOSTTOOLS_DIR)/$(CMS2BBF_APP)

cms2bbf_build:
ifneq ($(strip $(BUILD_PROFILE_SUPPORTED_DATA_MODEL)),)
ifneq ($(wildcard $(CMS2BBF_DIR)/Makefile),)
	$(MAKE) -C hostTools build_cms2bbf
else
	@echo "Skip $(CMS2BBF_APP) (sources not found)"
endif
else
	@echo "Skip $(CMS2BBF_APP) (not configured)"
endif

data-model:  cms2bbf_build
	$(MAKE) -C data-model

unittests:
	$(MAKE) -C unittests

unittests_run:
	$(MAKE) -C unittests unittests_run

doxygen_build:
	$(MAKE) -C hostTools build_doxygen

doxygen_docs: doxygen_build
	rm -rf $(BUILD_DIR)/docs/doxygen;
	mkdir $(BUILD_DIR)/docs/doxygen;
	cd hostTools/doxygen/bin; ./doxygen

doxygen_clean:
	-$(MAKE) -C hostTools clean_doxygen



############################################################################
#
# Build user applications depending on if they are
# specified in the profile.  Most of these BUILD_ checks should eventually get
# moved down to the userspace directory.
#
############################################################################

ifneq ($(strip $(BUILD_VCONFIG)),)
export BUILD_VCONFIG=y
endif


ifneq ($(strip $(BUILD_GDBSERVER)),)
gdbserver:
	install -m 755 $(TOOLCHAIN_TOP)/usr/$(TOOLCHAIN_PREFIX)/target_utils/gdbserver $(INSTALL_DIR)/bin
else
gdbserver:
endif

ifneq ($(strip $(BUILD_ETHWAN)),)
export BUILD_ETHWAN=y
endif

ifneq ($(strip $(BUILD_4_LEVEL_QOS)),)
export BUILD_4_LEVEL_QOS=y
endif

ifneq ($(strip $(BUILD_BRCM_CPEROUTER)),)
hnd_dongle: version_info
ifneq ($(strip $(BUILD_HND_NIC))$(strip $(BUILD_43602NIC)),)
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP) PROFILE_FILE=$(PROFILE_FILE) version
else
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP) PROFILE_FILE=$(PROFILE_FILE) pciefd
endif
else
hnd_dongle:
	@true
endif


ifneq ($(strip $(BUILD_DIAGAPP)),)
diagapp:
	$(MAKE) -C $(BROADCOM_DIR)/diagapp $(BUILD_DIAGAPP)
else
diagapp:
endif



ifneq ($(strip $(BUILD_IPPD)),)
ippd:
	$(MAKE) -C $(BROADCOM_DIR)/ippd $(BUILD_IPPD)
else
ippd:
endif


ifneq ($(strip $(BUILD_PORT_MIRRORING)),)
export BUILD_PORT_MIRRORING=1
else
export BUILD_PORT_MIRRORING=0
endif

ifeq ($(BRCM_USE_SUDO_IFNOT_ROOT),y)
BRCM_BUILD_USR=$(shell whoami)
BRCM_BUILD_USR1=$(shell sudo touch foo;ls -l foo | awk '{print $$3}';sudo rm -rf foo)
else
BRCM_BUILD_USR=root
endif


# These are the main targets for building the hostTools
HOSTTOOLS_SET = hosttools_bootloader hosttools_kernelspace hosttools_userspace hosttools_image

$(HOSTTOOLS_SET): hosttools_common
	$(MAKE) -C $(HOSTTOOLS_DIR) $@

ifneq ($(strip $(PREBUILD_DONE)),y)
hosttools_common:
	$(MAKE) -C $(HOSTTOOLS_DIR) $@
else
hosttools_common:
endif

# Just to keep legacy target. No internal targets depends on hosttools anymore.
hosttools: $(HOSTTOOLS_SET)

hosttools_nandcfe:
	$(MAKE) -C $(HOSTTOOLS_DIR) perlmods mkjffs2 build_imageutil build_cmplzma build_secbtutils build_mtdutils

.PHONY: hosttools_common $(HOSTTOOLS_SET) hosttools_nandcfe
# needed by hostTools/Makefile
export BUILD_MODSW_EE


############################################################################
#
# IKOS defines
#
############################################################################

CMS_VERSION_FILE=$(BUILD_DIR)/userspace/public/include/version.h

############################################################################
#
# Generate the credits
#
############################################################################
gen_credits:
	cd $(RELEASE_DIR); \
	if [ -e gen_credits.pl ]; then \
	  perl gen_credits.pl; \
	fi


############################################################################
#
# Packages: is this just for SGS?
#
############################################################################
packages: | all_postcheck1
ifneq ($(strip $(BUILD_SGS)),)
	if [ -f $(BUILD_DIR)/userspace/private/apps/sgs/Bcmbuild.mk ]; then \
		make -C $(BUILD_DIR)/userspace/private/apps/sgs -f Bcmbuild.mk build_package; \
	fi
endif


############################################################################
#
# Final image build
#
############################################################################


#
# full_buildimage is building image WITH dependency checking (kernelbuild, $(DTBS) libcreduction, etc.
# buildimage is building image WITHOUT dependency checking (faster).
# Note this whole block of code, down to the "END full_buildimage buildimage" comment is basically one
# single target.  There are a lot of ifdefs and blank lines which makes it hard to read/misleading.
# If full_buildimage is used, the first one is executed, the dependencies are satisifed,
# and then the execution falls through to the second full_buildimage.
#
full_buildimage: kernelbuild $(DTBS) libcreduction gen_credits linux_tools

full_buildimage buildimage:
	@echo "=>BUILDIMAGE Starting...<="

ifeq ($(BUILD_DISABLE_EXEC_STACK),y)
ifneq ($(execstack_exec),)
	@echo no need to build execstack $(execstack_exec)
else
	make -C $(HOSTTOOLS_DIR) build_execstack;
endif
endif

ifneq ($(strip $(BUILD_SYSTEMD)),)
	@echo do systemd special part
	cd $(PROFILE_DIR) && rm -f special-buildFS && ln -s ../buildFS_SYSTEMD special-buildFS
endif

ifneq ($(strip $(BUILD_BRCM_OPENWRT)),)
	# Openwrt, including Native Openwrt builds and Eos, use Openwrt rootfs
	cd $(TARGETS_DIR); ./buildFSopenwrt;
else
	# Normal BDK builds use our BDK rootfs/buildFS
	cd $(TARGETS_DIR); ./buildFS;
endif

ifneq ($(BUILD_MODSW_EXAMPLEEE),)
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmbeep.mk buildFS_EXAMPLEEE
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmbeep.mk buildFS_EXAMPLEEE2
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmbeep.mk buildFS_EXAMPLEEE3
endif
ifneq ($(BUILD_MODSW_DOCKEREE),)
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmbeep.mk buildFS_DOCKEREE
endif

ifneq ($(BUILD_MODSW_EE),)
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmbeep.mk buildFS_OPSPREINSTALL
	cd $(TARGETS_DIR); ./buildFS;
endif

ifeq ($(strip $(BRCM_RAMDISK_BOOT_EN)),y)
	cd $(TARGETS_DIR); ./buildFS_RD
endif

	cd $(TARGETS_DIR); ./buildROOTFS

ifneq ($(BUILD_UBOOT),)
	$(MAKE) -f build/make.uboot image_linux INCLUDE_OPTEE=$(BCM_OPTEE) INCLUDE_ATF=$(BCM_ARMTF) IMAGE_GOAL=$(IMAGE_GOAL)
endif

	@mkdir -p $(IMAGES_DIR)

# END OF full_buildimage buildimage target


.PHONY : uboot
uboot:
ifneq ($(BUILD_UBOOT),)
	rm -f bootloaders/obj/binaries/*.pkgtb
	$(MAKE) -f build/make.uboot all INCLUDE_OPTEE=$(BCM_OPTEE) INCLUDE_ATF=$(BCM_ARMTF) IMAGE_GOAL=$(IMAGE_GOAL)
endif

prepare_linux_image: kernelbuild hosttools_image
	cd $(TARGETS_DIR); ./buildFS vmlinux
ifneq ($(BUILD_UBOOT),)
	$(MAKE) -f build/make.uboot $@ INCLUDE_OPTEE=$(BCM_OPTEE) INCLUDE_ATF=$(BCM_ARMTF) IMAGE_GOAL=$(IMAGE_GOAL)
endif

shell:
	@echo "You are in a shell that includes the Makefile environment.  "exit" to return to normal"
	PS1='!_' bash --norc --noprofile


########################
#  BEEP package build  #
########################
-include $(BUILD_DIR)/make.beep


###########################################
#
# System code clean-up
#
###########################################
CLEAN_WITH_SANITY_CHECK :=

.PHONY : clean clean1 bcmdrivers_clean data-model_clean clean_with_sanity_check

# Keep clean1 just for Preflight
clean1: clean

clean: bcmdrivers_clean data-model_clean dynamic_cfe_clean optee_clean atf_clean\
	rdp_clean $(DTBS_CLEAN) clean_with_sanity_check openwrt_clean opensync_gw_clean openjdk_clean\
	clean_bcmdrivers_autogen uboot_clean
	rm -f $(HOSTTOOLS_DIR)/scripts/lxdialog/*.o
	rm -f .tmpconfig*
	-mv -f $(LAST_PROFILE_COOKIE) .check_clean
	rm -f $(LAST_PROFILE_COOKIE)
	rm -f $(IMAGE_GOAL_COOKIE)
	rm -f $(HOST_PERLARCH_COOKIE)
	rm -f bcmdrivers/broadcom/char/adsl/impl1/adsl_phy.bin
	rm -f bcmdrivers/broadcom/char/adsl/impl1/adslcore*/AdslPhyBld

uboot_clean:
	$(MAKE) -f build/make.uboot clean

dynamic_cfe_clean:
	-rm -rf cfe/build/broadcom/build_cfe*
	-rm  -f cfe/build/broadcom/bcm63xx_rom/*.S
	-rm  -f cfe/build/broadcom/bcm63xx_rom/*.cferamlz

cleanall: clean_local_tools clean

clean_local_tools:
	rm -rf $(HOSTTOOLS_DIR)/local_install

check_clean:
	find . -type f -newer .check_clean -print | $(HOSTTOOLS_DIR)/check_clean.pl -p .check_clean check_clean_whitelist

fssrc_clean:
	rm -fr $(FSSRC_DIR)/bin
	rm -fr $(FSSRC_DIR)/sbin
	rm -fr $(FSSRC_DIR)/lib
	rm -fr $(FSSRC_DIR)/upnp
	rm -fr $(FSSRC_DIR)/docs
	rm -fr $(FSSRC_DIR)/webs
	rm -fr $(FSSRC_DIR)/usr
	rm -fr $(FSSRC_DIR)/linuxrc
	rm -fr $(FSSRC_DIR)/images
	rm -fr $(FSSRC_DIR)/etc/wlan
	rm -fr $(FSSRC_DIR)/etc/certs

CLEAN_WITH_SANITY_CHECK += kernel_clean

kernel_clean: sanity_check hnd_dongle_clean
	- TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk mrproper clean
	rm -f $(KERNEL_DIR)/arch/arm/defconfig
	rm -f $(KERNEL_DIR)/arch/arm64/defconfig
	rm -f $(HOSTTOOLS_DIR)/lzma/decompress/*.o
	rm -f $(KERNEL_INCLUDE_LINK)
	rm -f $(KERNEL_ARM_INCLUDE_LINK)
	# $(KERNEL_DIR)/.pre_kernelbuild is generated by pre_kernelbuild.mk
	rm -f $(KERNEL_DIR)/.pre_kernelbuild
	rm -f $(PRE_KERNELBUILD)
	rm -f $(KERNELLINKS)
	rm -f $(KERNEL_DIR)/*_flags.txt
	rm -f $(BCM_SWVERSION_FILE)
	rm -f $(INC_KERNEL_BASE)/.menuconfig.log
ifneq ($(BCM_WLIMPL),)
ifeq ($(strip $(BUILD_BRCM_CPEROUTER)),)
	-find bcmdrivers/broadcom/net/wl/impl$(BCM_WLIMPL) -name build -type d -prune -exec rm -rf {} \; 2> /dev/null
else
	-rm -f bcmdrivers/broadcom/net/wl/impl$(BCM_WLIMPL)/Makefile
endif
endif
ifneq ($(strip $(BUILD_LINUX_PERF)),)
	-$(MAKE) -C $(KERNEL_DIR)/tools/perf clean
endif

bcmdrivers_clean:
	-$(MAKE) -C bcmdrivers clean

CLEAN_WITH_SANITY_CHECK += userspace_clean
userspace_clean: sanity_check fssrc_clean
	-rm -f $(BUILD_DIR)/cflags_snapshot.h
	-$(MAKE) -C userspace clean

ifeq ($(strip $(BUILD_BRCM_OPENWRT)),)
openwrt_clean:
	@echo "SKIPPING ALTSDK OPENWRT CLEAN!"
else
openwrt_clean:
	-$(MAKE) -C altsdk/openwrt clean
endif

ifeq ($(strip $(BUILD_OPENSYNC_GW)),)
opensync_gw_clean:
	@echo "SKIPPING OPENSYNC GW CLEAN!"
else
opensync_gw_clean:
	-$(MAKE) -C altsdk/atom -f Makefile.bdk clean
endif

ifeq ($(strip $(BUILD_OPENJDK)),)
openjdk_clean:
	@echo "SKIPPING OPENJDK CLEAN!"
else
openjdk_clean:
	-$(MAKE) -C altsdk/openjdk clean
endif

data-model_clean:
	-$(MAKE) -C data-model clean

unittests_clean:
	-$(MAKE) -C unittests clean

CLEAN_WITH_SANITY_CHECK += target_clean
target_clean: sanity_check userspace_clean
	rm -f $(PROFILE_DIR)/*.img
	rm -f $(PROFILE_DIR)/*.img.raw
	rm -f $(PROFILE_DIR)/*.bin
	rm -f $(PROFILE_DIR)/*.ini
	rm -f $(PROFILE_DIR)/rootfs*.ubifs
	rm -f $(PROFILE_DIR)/rootfs.ext4
	rm -f $(PROFILE_DIR)/bcm*rootfs.ext4
	rm -f $(PROFILE_DIR)/vmlinux*
	rm -f $(PROFILE_DIR)/*.w
	rm -f $(PROFILE_DIR)/*.pkgtb
	rm -f $(PROFILE_DIR)/*.itb
	rm -f $(PROFILE_DIR)/*ubifs
	rm -f $(PROFILE_DIR)/*.squashfs
	rm -f $(PROFILE_DIR)/*.gz
	rm -f $(PROFILE_DIR)/*.srec
	rm -f $(PROFILE_DIR)/ramdisk
	rm -f $(PROFILE_DIR)/$(FS_KERNEL_IMAGE_NAME)*
	rm -f $(PROFILE_DIR)/$(CFE_FS_KERNEL_IMAGE_NAME)*
	rm -f $(PROFILE_DIR)/$(FLASH_IMAGE_NAME)*
	rm -fr $(PROFILE_DIR)/modules
	rm -fr $(PROFILE_DIR)/imagebuild/
	rm -fr $(PROFILE_DIR)/op
	-rm -f $(PROFILE_DIR)/image_ident
	rm -fr $(INSTALL_DIR)
	rm -fr $(BCM_FSBUILD_DIR)
	-find targets -name vmlinux -print -exec rm -f "{}" ";"
	rm -fr targets/TEMP
	rm -fr $(TARGET_FS)
	rm -f release/*credits.txt
	rm -f $(CREATE_INSTALL)
ifeq ($(strip $(BRCM_KERNEL_ROOTFS)),all)
	rm -fr $(TARGET_BOOTFS)
endif

CLEAN_WITH_SANITY_CHECK += hosttools_clean	# for libcreduction clean
hosttools_clean:
	-$(MAKE) -C $(HOSTTOOLS_DIR) clean

.PHONY : hnd_dongle_clean
hnd_dongle_clean:
ifneq ($(strip $(BUILD_BRCM_CPEROUTER)),)
	# need to make sure soft link still exists
	-[ ! -d $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP) ] || $(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP) PROFILE_FILE=$(PROFILE_FILE) clean
endif

.PHONY : $(CLEAN_WITH_SANITY_CHECK)
ifneq ($(strip $(PROFILE)),)
clean_with_sanity_check : $(CLEAN_WITH_SANITY_CHECK)
clean_with_sanity_check : FORCE := 1
else
clean_with_sanity_check :
	$(warning PROFILE undefined, SKIPPED:$(CLEAN_WITH_SANITY_CHECK))
endif
###########################################
# End of system code clean-up
###########################################

arm8_srec_prepare:
	$(KOBJCOPY) --output-target=srec --input-target=binary --change-addresses=0x1fff000 kernel/dts/9$(BRCM_CHIP).dtb kernel/dts/9$(BRCM_CHIP)_dtb.srec;
	$(KOBJCOPY) --output-target=srec --input-target=binary --change-addresses=0x1b00000 $(PROFILE_DIR)/ramdisk $(PROFILE_DIR)/ramdisk.srec;
	$(KOBJCOPY) --output-target=srec $(PROFILE_DIR)/vmlinux $(PROFILE_DIR)/vmlinux.srec;

###########################################
#
# Temporary kernel patching mechanism
#
###########################################

.PHONY: genpatch patch

genpatch:
	@hostTools/kup_tmp/genpatch

patch:
#	@hostTools/kup_tmp/patch

###########################################
#
# Get modules version
#
###########################################
.PHONY: version_info SECUREHDR

version_info: sanity_check $(PRE_KERNELBUILD)
	@echo "$(MAKECMDGOALS):";\
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk --silent version_info;

###########################################
#
# System-wide exported variables
# (in alphabetical order)
#
###########################################

export \
ACTUAL_MAX_JOBS            \
BRCMAPPS                   \
BRCM_BOARD                 \
BRCM_DRIVER_PCI            \
BRCM_EXTRAVERSION          \
BRCM_KERNEL_NETQOS         \
BRCM_KERNEL_ROOTFS         \
BRCM_KERNEL_AUXFS_JFFS2    \
BRCM_CPU_FREQ_PWRSAVE      \
BRCM_CPU_FREQ_TARGET_LOAD  \
BRCM_PSI_VERSION           \
BRCM_PTHREADS              \
BRCM_RAMDISK_BOOT_EN       \
BRCM_RAMDISK_SIZE          \
BRCM_NFS_MOUNT_EN          \
BRCM_RELEASE               \
BRCM_RELEASETAG            \
BRCM_SNMP                  \
BRCM_VERSION               \
BUILD_CMFD                 \
BUILD_XDSLCTL              \
BUILD_XTMCTL               \
BUILD_VLANCTL              \
BUILD_BRCM_VLAN            \
BUILD_BRCTL                \
BUILD_DDNSD                \
BUILD_DEBUG_TOOLS          \
BUILD_DIAGAPP              \
BUILD_DIR                  \
BUILD_EBTABLES             \
BUILD_EPITTCP              \
BUILD_ETHWAN               \
BUILD_FTPD                 \
BUILD_FTPD_STORAGE         \
BUILD_WLHSPOT              \
BUILD_IPPD                 \
BUILD_IPROUTE2             \
BUILD_IPSEC_TOOLS          \
BUILD_L2TPAC               \
BUILD_ACCEL_PPTP           \
BUILD_WPS_BTN              \
BUILD_WSC                  \
BUILD_MKSQUASHFS           \
BUILD_NAS                  \
BUILD_PORT_MIRRORING       \
BUILD_PPPD                 \
PPP_AUTODISCONN            \
BUILD_SES                  \
BUILD_SIPROXD              \
BUILD_SLACTEST             \
BUILD_SNMP                 \
BUILD_SOAP                 \
BUILD_SOAP_VER             \
BUILD_SSHD                 \
BUILD_SSHD_MIPS_GENKEY     \
BUILD_TOD                  \
BUILD_BRCM_CMS             \
BUILD_TR69C                \
BUILD_TR69C_SSL            \
BUILD_OMCI                 \
BUILD_UDHCP                \
BUILD_UDHCP_RELAY          \
BUILD_VCONFIG              \
BUILD_SUPERDMZ             \
BUILD_WLCTL                \
BUILD_DHDCTL               \
BUILD_ZEBRA                \
BUILD_LIBUSB               \
BUILD_WANVLANMUX           \
HOSTTOOLS_DIR              \
INC_KERNEL_BASE            \
INSTALL_DIR                \
PROFILE_DIR                \
WEB_POPUP                  \
BUILD_VIRT_SRVR            \
BUILD_PORT_TRIG            \
BUILD_IPV6                 \
BUILD_BOARD_LOG_SECTION    \
BRCM_LOG_SECTION_SIZE      \
BRCM_FLASHBLK_SIZE         \
BRCM_AUXFS_PERCENT         \
BRCM_BACKUP_PSI            \
BUILD_IPSEC                \
BUILD_GPON                 \
BUILD_GPONCTL              \
BUILD_PMON                 \
BUILD_BUZZZ                \
BUILD_BOUNCE               \
BUILD_HELLO                \
BUILD_SPUCTL               \
RELEASE_BUILD              \
NO_PRINTK_AND_BUG          \
FLASH_NAND_BLOCK_128KB     \
FLASH_NAND_BLOCK_256KB     \
FLASH_NAND_BLOCK_512KB     \
FLASH_NAND_BLOCK_1024KB     \
BRCM_CONFIG_HIGH_RES_TIMERS \
BRCM_SWITCH_SCHED_SP        \
BRCM_SWITCH_SCHED_WRR       \
BUILD_IQCTL                 \
BUILD_BPMCTL                \
BUILD_EPONCTL               \
BUILD_ETHTOOL               \
BUILD_TMS                   \
TOOLCHAIN_PREFIX            \
PROFILE_KERNEL_VER          \
KERNEL_LINKS_DIR            \
LINUX_VER_STR               \
KERNEL_DIR                  \
FORCE                       \
BUILD_DPI                   \
BUILD_MAP                   \
BUILD_BRCM_CMS              \
BUILD_WEB_SOCKETS           \
BUILD_DISABLE_EXEC_STACK    \
BUILD_DBUS                  \
BUILD_LXC                   \
NO_MINIFY                   \
BRCM_PARTITION_CFG_FILE     \
BCM_SPEEDYGET               \
BUILD_VXLAN_TUNNEL          \
BUILD_MACSEC                \
BCM_QEMU_SIM                \
SKIP_RTPOLICY               \
BRCM_GLIBC

unexport BUILD_BRCM_CMS
###########################################
# End of the real part
###########################################



