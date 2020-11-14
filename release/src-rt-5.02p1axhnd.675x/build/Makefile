#***********************************************************************
#
#  Copyright (c) 2004  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/
export MY_DEFAULT_ANY_FIRST_RUN := 0
export SKIP_TIMESTAMP_IMAGE := y

# Top-level Makefile     
show_vars = $(info $(foreach v,$1,$v='$(value $v)'))
$(call show_vars,MAKELEVEL MAKEFLAGS MAKECMDGOALS MAKEOVERRIDES MAKEFILE_LIST)
$(call show_vars,MY_DEFAULT_ANY_FIRST_RUN MY_MKENV_FIRST_RECURSION)

###########################################
# Start of the real part
###########################################
###########################################
# This is the first target in the Makefile,
# so it is also the default target.
############################################

default: mkenv prebuild_checks all_postcheck1

all:
	$(MAKE) -f build/Makefile default

all_postcheck1: profile_saved_check sanity_check rdp_link\
     create_install pinmuxcheck dynamic_cfe\
     parallel_targets

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
build_cfe_nand : | build_cfe_emmc
build_cfe_emmc : | build_cfe_sec_nand
build_cfe_sec_nand : | build_cfe_sec_emmc

prebuild_checks : mkenv 

profile_saved_check : prebuild_checks

sanity_check : profile_saved_check

rdp_link create_install pinmuxcheck kernelbuild : sanity_check

modbuild : kernelbuild rdp_link

parallel_targets : modbuild create_install

gdbserver : create_install

ifneq ($(FB),y)
buildimage dtbs libcreduction gen_credits linux_tools : parallel_targets gdbserver kernelbuild
endif


.PHONY: mkenv all_postcheck1

# These post kernel top level targets can compile concurrently
parallel_targets: hosttools userspace optee

__parallel_targets: userspace hosttools 

dynamic_cfe: hosttools

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

ifeq ($(strip $(MAKECMDGOALS)),)
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
#
# Make info for voice
#
############################################################################
ifneq ($(strip $(BRCM_VOICE_SUPPORT)),)
export BRCM_VOICE_SUPPORT
BRCM_VOICE_INCLUDE_MAKE_TARGETS=1
include $(BUILD_DIR)/make.voice
endif

############################################################################
#
# Make info for RDP modules
#
############################################################################

rdp_link:
ifneq ($(strip $(RDP_PROJECT)),)
	$(shell echo $(INC_RDP_FLAGS) > $(KERNEL_DIR)/rdp_flags.txt)
	$(MAKE) -C $(RDPSDK_DIR) PROJECT=$(RDP_PROJECT) rdp_link
endif


rdp_clean:
ifneq ($(strip $(RDP_PROJECT)),)
	$(MAKE) -C $(RDPSDK_DIR) PROJECT=$(RDP_PROJECT) clean
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
export BUILD_ARMTF
optee: kernelbuild
	$(MAKE) -C $(BUILD_DIR)/secureos optee_os
optee_clean:
	$(MAKE) -C $(BUILD_DIR)/secureos clean
else
optee:
optee_clean:
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

#Set up ADSL_PLN_TEST
export ADSL_PLN_TEST=$(BUILD_TR69_XBRCM)

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




############################################################################
#
# When there is a directory name with the same name as a Make target,
# make gets confused.  PHONY tells Make to ignore the directory when
# trying to make these targets.
#
############################################################################
.PHONY: userspace unittests hostTools kernellinks kernelbuild pre_kernelbuild

#
# create a bcm_relversion.h which has our release version number, e.g.
# 4 10 02.  This allows device drivers which support multiple releases
# with a single driver image to test for version numbers.
#
BCM_SWVERSION_FILE := $(KERNEL_DIR)/include/linux/bcm_swversion.h
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
BCM_KF_KCONFIG_FILE := $(KERNEL_DIR)/Kconfig.bcm_kf
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




prepare_userspace: sanity_check create_install $(BCM_SWVERSION_FILE) kernellinks rdp_link hosttools 
	$(MAKE) -C userspace public-include

.PHONY: prepare_userspace

ifneq ($(strip $(SKIP_USERSPACE)),)
userspace:
	@echo "SKIPPING USERSPACE BUILD!"
else
userspace: prepare_userspace modbuild
	@echo "USERSPACE STARTED"
	$(MAKE) -C userspace
	@echo "USERSPACE ENDED"
endif

#
# Always run Make in the libcreduction directory.  In most non-voice configs,
# mklibs.py will be invoked to analyze user applications
# and libraries to eliminate unused functions thereby reducing image size.
# However, for voice configs, gdb server, oprofile and maybe some other
# special cases, the libcreduction makefile will just copy unstripped
# system libraries to fs.install for inclusion in the image.
#
libcreduction:
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
create_install:
		mkdir -p $(PROFILE_DIR)/fs.install/etc
		mkdir -p $(INSTALL_DIR)/bin
		mkdir -p $(INSTALL_DIR)/lib
		mkdir -p $(INSTALL_DIR)/etc/snmp
		mkdir -p $(INSTALL_DIR)/etc/iproute2
		mkdir -p $(INSTALL_DIR)/rom/bin
		mkdir -p $(INSTALL_DIR)/rom/modules
		mkdir -p $(INSTALL_DIR)/rom/scripts

.PHONY: create_install

kernellinks: $(KERNEL_INCLUDE_LINK) $(KERNEL_MIPS_INCLUDE_LINK) $(KERNEL_ARM_INCLUDE_LINK)

$(KERNEL_INCLUDE_LINK):
	ln -s -f $(KERNEL_DIR)/$(INC_DIR) $(KERNEL_INCLUDE_LINK);

$(KERNEL_MIPS_INCLUDE_LINK):
	ln -s -f $(KERNEL_DIR)/arch/mips/$(INC_DIR) $(KERNEL_MIPS_INCLUDE_LINK);

$(KERNEL_ARM_INCLUDE_LINK):
	ln -s -f $(KERNEL_DIR)/arch/arm/$(INC_DIR) $(KERNEL_ARM_INCLUDE_LINK);


.PHONY: bcmdrivers_autogen clean_bcmdrivers_autogen


BCMD_AG_MAKEFILE:=Makefile.autogen
BCMD_AG_KCONFIG:=Kconfig.autogen
BCMD_AG_MAKEFILE_TMP:=$(BCMD_AG_MAKEFILE).tmp
BCMD_AG_KCONFIG_TMP:=$(BCMD_AG_KCONFIG).tmp

bcmdrivers_autogen:
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

clean1: clean_bcmdrivers_autogen

clean_bcmdrivers_autogen: kernel_clean
	rm -f $(BRCMDRIVERS_DIR)/$(BCMD_AG_MAKEFILE_TMP) $(BRCMDRIVERS_DIR)/$(BCMD_AG_KCONFIG_TMP) $(BRCMDRIVERS_DIR)/$(BCMD_AG_MAKEFILE) $(BRCMDRIVERS_DIR)/$(BCMD_AG_KCONFIG)


.PHONY: bcmdrivers_autogen kernellinks

pre_kernelbuild: $(KERNEL_DIR)/.pre_kernelbuild;

ifdef BCM_KF
$(KERNEL_DIR)/.pre_kernelbuild: $(BCM_SWVERSION_FILE) $(BCM_KF_KCONFIG_FILE) bcmdrivers_autogen kernellinks 
else
$(KERNEL_DIR)/.pre_kernelbuild: $(BCM_KF_KCONFIG_FILE)
endif
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -f build/pre_kernelbuild.mk

ifdef BCM_KF
kernelbuild: $(KERNEL_DIR)/.pre_kernelbuild hnd_dongle rdp_link
else
kernelbuild: $(KERNEL_DIR)/.pre_kernelbuild
endif
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) $(PARALLEL_BUILD) -C $(BUILD_DIR)/build -f Bcmkernel.mk


headers_install: kernelbuild
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) $(PARALLEL_BUILD) -C $(BUILD_DIR)/build -f Bcmkernel.mk headers_install INSTALL_HDR_PATH=$(INC_KERNELAPI_PATH)

modbuild: headers_install

.PHONY: headers_install



linux_tools: linux_tools_perf

ifneq ($(strip $(BUILD_LINUX_PERF)),)
linux_tools_perf: $(KERNEL_DIR)/.pre_kernelbuild
	$(MAKE) -C $(KERNEL_DIR)/tools/perf WERROR=0
	install -m 755 $(KERNEL_DIR)/tools/perf/perf $(INSTALL_DIR)/bin
else
linux_tools_perf:
endif


kernel_config_test: $(KERNEL_DIR)/.pre_kernelbuild
	@echo
	@echo "Building $(DIR)/config_$(PROFILE)";
	-@mkdir $(DIR) 2> /dev/null || true
	sort $(KERNEL_DIR)/.config | grep -v "^\#.*$$" | grep -v "^[[:space:]]*$$" > $(DIR)/config_$(PROFILE)
	@echo "  ... done building $(DIR)/config_$(PROFILE)";

.PHONY: kernel_config_test

ifneq ($(findstring $(strip $(KERNEL_ARCH)),aarch64 arm mips mipsel),)
.PHONY:dtbs
dtbs: bcmdrivers_autogen  $(KERNEL_DIR)/.pre_kernelbuild
	@echo "Build dts for chip $(BRCM_CHIP)... "
	@echo "CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk boot=$(DTS_DIR)  dtbs"
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk boot=$(DTS_DIR)  dtbs
DTBS := dtbs

.PHONY:dtbs_clean
dtbs_clean:
	@echo "Clean dts for chip $(BRCM_CHIP)... "
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(DTS_DIR)/dts/$(BRCM_CHIP) dtbs_clean 
DTBS_CLEAN := dtbs_clean
else
DTBS := 
DTBS_CLEAN :=
endif


kernel: sanity_check create_install kernelbuild hosttools buildimage

shared_link:
	if [ ! -L '$(SRCBASE)/router/hnd_shared' ]; then \
		ln -sf $(SRCBASE)/shared  $(SRCBASE)/router/hnd_shared; \
	fi
	if [ ! -L '$(SRCBASE)/router-sysdep/hnd_shared' ]; then \
		ln -sf $(SRCBASE)/shared  $(SRCBASE)/router-sysdep/hnd_shared; \
	fi

modbuild: shared_link
	rm -rf $(HND_SRC)/targets/$(PROFILE)/modules
	@echo "******************** Starting modbuild ********************";
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk modules && \
	    CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk modules_install INSTALL_MOD_PATH=$(PROFILE_DIR)/modules
	@echo "******************** DONE modbuild ********************";

mocamodbuild:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_MOCACFGDRV_PATH) modules 
mocamodclean:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_MOCACFGDRV_PATH) clean

adslmodbuild:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_ADSLDRV_PATH) modules 
adslmodbuildclean:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_ADSLDRV_PATH) clean

spumodbuild:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_SPUDRV_PATH) modules
spumodbuildclean:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_SPUDRV_PATH) clean

pwrmngtmodbuild:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_PWRMNGTDRV_PATH) modules
pwrmngtmodclean:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_PWRMNGTDRV_PATH) clean

enetmodbuild:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_ENETDRV_PATH) modules
enetmodclean:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_ENETDRV_PATH) clean

.PHONY: modbuild mocamodbuild adslmodbuild spumodbuild pwrmngtmodbuild enetmodbuild modules eponmodbuild gponmodbuild adslmodule

eponmodbuild:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_EPONDRV_PATH) modules
eponmodclean:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_EPONDRV_PATH) clean

gponmodbuild:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_GPON_PATH) modules
gponmodclean:
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk M=$(INC_GPON_PATH) clean

modules: sanity_check create_install modbuild hosttools buildimage

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
ifeq ($(strip $(BUILD_BRCM_HNDROUTER_ALONE)),)
	$(MAKE) -C data-model
else
	# skip for HND router builds
	@true
endif

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

ifneq ($(strip $(BCA_HNDROUTER)),)
hnd_dongle: version_info
ifneq ($(strip $(BUILD_HND_NIC)),)
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP) version
else 
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP) pciefd
endif
else
hnd_dongle:
	@true
endif

# Leave it for the future when soap server is decoupled from cfm
ifneq ($(strip $(BUILD_SOAP)),)
ifeq ($(strip $(BUILD_SOAP_VER)),2)
soapserver:
	$(MAKE) -C $(BROADCOM_DIR)/SoapToolkit/SoapServer $(BUILD_SOAP)
else
soap:
	$(MAKE) -C $(BROADCOM_DIR)/soap $(BUILD_SOAP)
endif
else
soap:
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

hosttools:
	$(MAKE) -C $(HOSTTOOLS_DIR)

hosttools_nandcfe:
	$(MAKE) -C $(HOSTTOOLS_DIR) perlmods mkjffs2 build_imageutil build_cmplzma build_secbtutils build_mtdutils

.PHONY: hosttools hosttools_nandcfe

############################################################################
#
# IKOS defines
#
############################################################################

CMS_VERSION_FILE=$(BUILD_DIR)/userspace/public/include/version.h

ifeq ($(strip $(BRCM_IKOS)),y)
FS_COMPRESSION=-noD -noI -no-fragments
else
FS_COMPRESSION=
endif

export BRCM_IKOS FS_COMPRESSION



# IKOS Emulator build that does not include the CFE boot loader.
# Edit targets/ikos/ikos and change the chip and board id to desired values.
# Then build: make PROFILE=ikos ikos
ikos:
	@echo -e '#define SOFTWARE_VERSION ""\n#define RELEASE_VERSION ""\n#define PSI_VERSION ""\n' > $(CMS_VERSION_FILE)
	@-mv -f $(FSSRC_DIR)/etc/profile $(FSSRC_DIR)/etc/profile.dontuse >& /dev/null
	@-mv -f $(FSSRC_DIR)/etc/init.d $(FSSRC_DIR)/etc/init.dontuse >& /dev/null
	@-mv -f $(FSSRC_DIR)/etc/inittab $(FSSRC_DIR)/etc/inittab.dontuse >& /dev/null
	@sed -e 's/^::respawn.*sh.*/::respawn:-\/bin\/sh/' $(FSSRC_DIR)/etc/inittab.dontuse > $(FSSRC_DIR)/etc/inittab
	@if [ ! -a $(CFE_FILE) ] ; then echo "no cfe" > $(CFE_FILE); echo "no cfe" > $(CFE_FILE).del; fi
	@-rm $(HOSTTOOLS_DIR)/bcmImageBuilder >& /dev/null
	$(MAKE) -f build/Makefile PROFILE=$(PROFILE)
	@-rm $(HOSTTOOLS_DIR)/bcmImageBuilder >& /dev/null
	@mv -f $(FSSRC_DIR)/etc/profile.dontuse $(FSSRC_DIR)/etc/profile
	@-mv -f $(FSSRC_DIR)/etc/init.dontuse $(FSSRC_DIR)/etc/init.d >& /dev/null
	@-mv -f $(FSSRC_DIR)/etc/inittab.dontuse $(FSSRC_DIR)/etc/inittab >& /dev/null
	@cd $(PROFILE_DIR); \
	$(KOBJCOPY) --output-target=srec vmlinux vmlinux.srec; \
	xxd $(FS_KERNEL_IMAGE_NAME) | grep "^00000..:" | xxd -r > bcmtag.bin; \
	$(KOBJCOPY) --output-target=srec --input-target=binary --change-addresses=0xb8010000 bcmtag.bin bcmtag.srec; \
	$(KOBJCOPY) --output-target=srec --input-target=binary --change-addresses=0xb8010100 rootfs.img rootfs.srec; \
	rm bcmtag.bin; \
	grep -v "^S7" vmlinux.srec > bcm9$(BRCM_CHIP)_$(PROFILE).srec; \
	grep "^S3" bcmtag.srec >> bcm9$(BRCM_CHIP)_$(PROFILE).srec; \
	grep -v "^S0" rootfs.srec >> bcm9$(BRCM_CHIP)_$(PROFILE).srec
	@if [ ! -a $(CFE_FILE).del ] ; then rm -f $(CFE_FILE) $(CFE_FILE).del; fi
	@echo -e "\nAn image without CFE for the IKOS emulator has been built.  It is named"
	@echo -e "targets/$(PROFILE)/bcm9$(BRCM_CHIP)_$(PROFILE).srec\n"

# IKOS Emulator build that includes the CFE boot loader.
# Both Linux and CFE boot loader toolchains need to be installed.
# Edit targets/ikos/ikos and change the chip and board id to desired values.
# Then build: make PROFILE=ikos ikoscfe
ikoscfe:
	@echo -e '#define SOFTWARE_VERSION ""\n#define RELEASE_VERSION ""\n#define PSI_VERSION ""\n' > $(CMS_VERSION_FILE)
	@-mv -f $(FSSRC_DIR)/etc/profile $(FSSRC_DIR)/etc/profile.dontuse >& /dev/null
	$(MAKE) -f build/Makefile PROFILE=$(PROFILE)
	@mv -f $(FSSRC_DIR)/etc/profile.dontuse $(FSSRC_DIR)/etc/profile
	$(MAKE) -C $(BL_BUILD_DIR) clean
	$(MAKE) -C $(BL_BUILD_DIR)
	$(MAKE) -C $(BL_BUILD_DIR) ikos_finish
	cd $(PROFILE_DIR); \
	echo -n "** no kernel  **" > kernelfile; \
	$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(CFE_FS_KERNEL_IMAGE_NAME) --chip $(BRCM_CHIP) --board $(BRCM_BOARD_ID) --blocksize $(BRCM_FLASHBLK_SIZE) --cfefile $(BL_BUILD_DIR)/cfe$(BRCM_CHIP).bin --rootfsfile rootfs.img --kernelfile kernelfile --dtbfile $(DTB_FILE) --include-cfe; \
	$(HOSTTOOLS_DIR)/createimg.pl --set boardid=$(BRCM_BOARD_ID) voiceboardid=$(BRCM_VOICE_BOARD_ID) numbermac=$(BRCM_NUM_MAC_ADDRESSES) macaddr=$(BRCM_BASE_MAC_ADDRESS) tp=$(BRCM_MAIN_TP_NUM) psisize=$(BRCM_PSI_SIZE) --inputfile=$(CFE_FS_KERNEL_IMAGE_NAME) --outputfile=$(FLASH_IMAGE_NAME) --nvramfile $(HOSTTOOLS_DIR)/nvram.h --nvramdefsfile $(HOSTTOOLS_DIR)/nvram_defaults.h --config=$(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf;\
	$(HOSTTOOLS_DIR)/addvtoken --endian $(ARCH_ENDIAN) $(FLASH_IMAGE_NAME) $(FLASH_IMAGE_NAME).w; \
	$(KOBJCOPY) --output-target=srec --input-target=binary --change-addresses=0xb8000000 $(FLASH_IMAGE_NAME).w $(FLASH_IMAGE_NAME).srec; \
	$(KOBJCOPY) --output-target=srec vmlinux vmlinux.srec; \
	@rm kernelfile; \
	grep -v "^S7" vmlinux.srec > bcm9$(BRCM_CHIP)_$(PROFILE).srec; \
	grep "^S3" $(BL_BUILD_DIR)/cferam$(BRCM_CHIP).srec >> bcm9$(BRCM_CHIP)_$(PROFILE).srec; \
	grep -v "^S0" $(FLASH_IMAGE_NAME).srec >> bcm9$(BRCM_CHIP)_$(PROFILE).srec; \
	grep -v "^S7" vmlinux.srec > bcm9$(BRCM_CHIP)_$(PROFILE).utram.srec; \
	grep -v "^S0" $(BL_BUILD_DIR)/cferam$(BRCM_CHIP).srec >> bcm9$(BRCM_CHIP)_$(PROFILE).utram.srec;
	@echo -e "\nAn image with CFE for the IKOS emulator has been built.  It is named"
	@echo -e "targets/$(PROFILE)/bcm9$(BRCM_CHIP)_$(PROFILE).srec"
	@echo -e "\nBefore testing with the IKOS emulator, this build can be unit tested"
	@echo -e "with an existing chip and board as follows."
	@echo -e "1. Flash targets/$(PROFILE)/$(FLASH_IMAGE_NAME).w onto an existing board."
	@echo -e "2. Start the EPI EDB debugger.  At the edbice prompt, enter:"
	@echo -e "   edbice> fr m targets/$(PROFILE)/bcm9$(BRCM_CHIP)_$(PROFILE).utram.srec"
	@echo -e "   edbice> r"
	@echo -e "3. Program execution will start at 0xb8000000 (or 0xbfc00000) and,"
	@echo -e "   if successful, will enter the Linux shell.\n"


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
# PinMuxCheck
#
############################################################################
pinmuxcheck:
ifeq ($(wildcard $(HOSTTOOLS_DIR)/PinMuxCheck/Makefile),)
	@echo "No PinMuxCheck needed"
else
	cd $(HOSTTOOLS_DIR); $(MAKE) build_pinmuxcheck BUILD_DISABLE_PINMUXTEST=$(BUILD_DISABLE_PINMUXTEST) BP_PHYS_INTF=$(BP_PHYS_INTF);
endif

.PHONY: pinmuxcheck

############################################################################
#
# This is where we build the image
#
############################################################################

ifeq ($(strip $(SECURE_BOOT_HWA)),y)
SECUREHDR:=--securehdr "$(SECURE_BOOT_HWA_OPT)" 
else
SECUREHDR:=--securehdr " "
endif

execstack_exec=$(shell which execstack)

# Emmc images are built for BOOTROM enabled devices only
ifeq ($(strip $(BUILD_EMMC_IMG)),y)
ifeq ($(strip $(BRCM_CHIP)),63138)
BLD_EMMC_BTROM_BOOT_IMAGES=y
else
ifeq ($(strip $(BTRM_BOOT_ONLY)),y)
BLD_EMMC_BTROM_BOOT_IMAGES=y
endif
endif
endif

# build_images_unsecure and build_images_secure functions takes 6 argmuments
# arg0 - function name, ignored  
# arg1 - concatination of B_$(BUILD_NAND_IMG_BLKSIZE_$(3)KB)_$(BRCM_FLASH_NAND_LAYOUT_SPLIT)_L_$(BUILD_NAND_IMG_BLKSIZE_$(3)KB)_$(BRCM_FLASH_NAND_LAYOUT_PUREUBI)
# 	3 above stands for the block size e.g. 128,256,512 and 1024
# arg2 - cferom file
# arg3 - block size
# arg4 - bootofs
# arg5 - precferom or securehdr
# arg6 - --unsecurehdr
# arg7 - u$(BRCM_FLASH_NAND_ROOTFS_UBIFS)
# arg8 - s$(BRCM_FLASH_NAND_ROOTFS_SQUBI)

BUILD_NAND_IMG_BLKSIZE_ALL := $(filter BUILD_NAND_IMG_BLKSIZE_%,$(.VARIABLES))
BUILD_NAND_IMG_BLKSIZE=$(foreach var,$(BUILD_NAND_IMG_BLKSIZE_ALL),$(if $(findstring $($(var)),y),$(subst KB,,$(subst BUILD_NAND_IMG_BLKSIZE_,,$(var)))))
#BRCM_FLASH_NAND_LAYOUT:= $(filter BRCM_FLASH_NAND_LAYOUT_%,$(.VARIABLES))
#BRCM_FLASH_NAND_ROOTFS := $(filter BRCM_FLASH_NAND_ROOTFS_%,$(.VARIABLES))

# setup the ddr type flag
ifeq ($(CFE_DDR_TYPE_CHECK),y)
ifeq ($(strip $(BUILD_CFE_DDR4)),y)
IMG_DDR_TYPE_OPT:=--ddrtype DDR4
else
IMG_DDR_TYPE_OPT:=--ddrtype DDR3
endif
endif


define build_images_unsecure
$(if $(findstring B_y, $(1)), \
$(if $(findstring uy, $(7)), \
      $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB)  --ubifs --bootfs bootfs$(3)kb.img --rootfs ubi_rootfs$(3)kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_ubi --fsonly $(FLASH_NAND_FS_IMAGE_NAME_$(3))_ubi $(IMG_DDR_TYPE_OPT) $(6) $(5);)\
$(if $(findstring sy, $(8)), \
       $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB)  --squbifs --bootfs bootfs$(3)kb.img --rootfs squbi_rootfs$(3)kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_squbi --fsonly $(FLASH_NAND_FS_IMAGE_NAME_$(3))_squbi $(IMG_DDR_TYPE_OPT) $(6) $(5);)\
  )
$(if $(findstring L_y, $(1)), \
$(if $(findstring uy, $(7)), \
       $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB)  --rootfs ubi_rootfs$(3)kb_pureubi.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi --fsonly $(FLASH_NAND_FS_IMAGE_NAME_$(3))_pureubi --ubionlyimage $(IMG_DDR_TYPE_OPT) $(6) $(5);)\
       md5sum $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi.w > $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi.md5; \
$(if $(LIVE_UPDATE_RSA_VER), \
$(if $(wildcard $(KEY_PATH)/private*.pem), \
       for f in $(KEY_PATH)/private*.pem; do \
              rsa_idx=$$(echo $${f} | sed "s/.*\///" | sed "s/private//" | sed "s/.pem//"); \
              $$(openssl sha1 -sign $(KEY_PATH)/private$${rsa_idx}.pem -out $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi_rsa$${rsa_idx}.zip $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi.w); \
       done \
, \
       if [ -f $(HND_SRC)/../../buildtools/private.pem ] ; then \
       openssl sha1 -sign $(HND_SRC)/../../buildtools/private.pem -out $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi_rsa.zip $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi.w; \
       fi \
)
, \
       if [ -f $(HND_SRC)/../../buildtools/private.pem ] ; then \
       openssl sha1 -sign $(HND_SRC)/../../buildtools/private.pem -out $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi_rsa.zip $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi.w; \
       fi \
)
$(if $(findstring sy, $(8)), \
       $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB)  --rootfs squbi_rootfs$(3)kb_pureubi.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_puresqubi --fsonly $(FLASH_NAND_FS_IMAGE_NAME_$(3))_puresqubi --ubionlyimage $(IMG_DDR_TYPE_OPT) $(6) $(5);)\
       [ ! -f $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_FS_IMAGE_NAME_$(3))_puresqubi.w ] || md5sum $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_FS_IMAGE_NAME_$(3))_puresqubi.w > $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_FS_IMAGE_NAME_$(3))_puresqubi.md5; \
       [ ! -f $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_FS_IMAGE_NAME_$(3))_puresqubi.w ] || [ ! -f $(HND_SRC)/../../buildtools/private.pem ] || openssl sha1 -sign $(HND_SRC)/../../buildtools/private.pem -out $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_FS_IMAGE_NAME_$(3))_puresqubi_rsa.zip $(HND_SRC)/targets/$(PROFILE)/$(FLASH_NAND_FS_IMAGE_NAME_$(3))_puresqubi.w; \
  )
endef

define build_images_secure
$(if $(findstring B_y, $(1)), \
$(if $(findstring uy, $(6)),\
        $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --cfesecrom $(8) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB) --ubifs --bootfs bootfs$(3)kb_secureboot.img --rootfs ubi_rootfs$(3)kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_ubi_secureboot --fsonly $(FLASH_NAND_FS_IMAGE_NAME_$(3))_ubi_secureboot $(IMG_DDR_TYPE_OPT) $(5);)\
$(if $(findstring sy, $(7)),\
        $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --cfesecrom $(8) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB) --squbifs --bootfs bootfs$(3)kb_secureboot.img --rootfs squbi_rootfs$(3)kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_squbi_secureboot --fsonly $(FLASH_NAND_FS_IMAGE_NAME_$(3))_squbi_secureboot $(IMG_DDR_TYPE_OPT) $(5);)\
  )
$(if $(findstring L_y, $(1)), \
# w/wo CFEROM + UBI CFERAM + UBI VMLINUX + UBIFS
$(if $(findstring uy, $(6)),\
        $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --cfesecrom $(8) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB) --bootfs bootfs$(3)kb_secureboot.img --rootfs ubi_rootfs$(3)kb_pureubi.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_pureubi_secureboot --fsonly $(FLASH_NAND_FS_IMAGE_NAME_$(3))_pureubi_secureboot $(IMG_DDR_TYPE_OPT) $(5);)\
$(if $(findstring sy, $(7)),\
        $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --cfesecrom $(8) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB) --bootfs bootfs$(3)kb_secureboot.img --rootfs squbi_rootfs$(3)kb_pureubi.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_$(3))_puresqubi_secureboot --fsonly $(FLASH_NAND_FS_IMAGE_NAME_$(3))_puresqubi_secureboot $(IMG_DDR_TYPE_OPT) $(5);)\
    )
endef


define build_cfeonly_images_unsecure
        $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB)  --rootfs rootfs$(3)kb.img --image $(FLASH_BASE_IMAGE_NAME)_nand_cfeonly.$(3) $(IMG_DDR_TYPE_OPT) $(6) $(5);\
        $(TARGETS_DIR)/buildUBI -u $(PROFILE_DIR)/ubi_cfe.ini -m $(TARGET_FS)/../metadata.bin -f $(PROFILE_DIR)/filestruct_cfe.bin -t $(TARGET_FS);\
        $(HOSTTOOLS_DIR)/mtd-utils*/ubinize -v -o $(PROFILE_DIR)/ubi_rootfs$(3)kb_cferam_pureubi.img -m 2048 -p $(FLASH_NAND_BLOCK_$(3)KB) $(PROFILE_DIR)/ubi_cfe.ini;\
        $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB)  --rootfs ubi_rootfs$(3)kb_cferam_pureubi.img --image $(FLASH_BASE_IMAGE_NAME)_nand_cfeonly_pureubi.$(3) $(IMG_DDR_TYPE_OPT) $(6) $(5);
endef



define build_cfeonly_images_secure
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --cfesecrom $(6) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB) --rootfs rootfs$(3)kb_secureboot.img --image $(FLASH_BASE_IMAGE_NAME)_nand_cfeonly_secureboot.$(3) $(IMG_DDR_TYPE_OPT) $(5);\
        $(TARGETS_DIR)/buildUBI -u $(PROFILE_DIR)/ubi_cfe.ini -m $(TARGET_FS)/../metadata.bin -f $(PROFILE_DIR)/filestruct_cfe.bin -t $(TARGET_FS);\
        $(HOSTTOOLS_DIR)/mtd-utils*/ubinize -v -o $(PROFILE_DIR)/ubi_rootfs$(3)kb_cferam_pureubi.img -m 2048 -p $(FLASH_NAND_BLOCK_$(3)KB) $(PROFILE_DIR)/ubi_cfe.ini;\
        $(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(2) --cfesecrom $(6) --blocksize $(FLASH_NAND_BLOCK_$(3)KB) --bootofs $(4) --bootsize $(FLASH_NAND_BLOCK_$(3)KB)  --rootfs ubi_rootfs$(3)kb_cferam_pureubi.img --image $(FLASH_BASE_IMAGE_NAME)_nand_cfeonly_pureubi_secureboot.$(3) $(IMG_DDR_TYPE_OPT) $(5);
endef

cfebuild: sanity_check
ifeq ($(strip $(SECURE_BOOT_TURNKEY)),y)
	@echo Start cfebuild
	if [ "$(FORCE)" == "1" -o ! -e $(LAST_PROFILE_COOKIE) -o   -e $(LAST_PROFILE_COOKIE) -a targets/cfe/cfe$(BRCM_CHIP)rom.bin -ot $(LAST_PROFILE_COOKIE) ]; then \
		$(MAKE) -C cfe/build/broadcom/bcm63xx_rom $(BRCM_CHIP)_SEC_TK; \
	fi
else
	@echo Start cfebuild
	if [ "$(FORCE)" == "1" -o ! -e $(LAST_PROFILE_COOKIE) -o   -e $(LAST_PROFILE_COOKIE) -a targets/cfe/cfe$(BRCM_CHIP)rom.bin -ot $(LAST_PROFILE_COOKIE) ]; then \
		$(MAKE) -C cfe/build/broadcom/bcm63xx_rom $(BRCM_CHIP); \
	fi
endif


ifneq ($(if $(wildcard cfe/cfe/board),$(BUILD_DYNAMIC_CFE)),)
dynamic_cfe: build_cfe_nand build_cfe_emmc build_cfe_sec_nand build_cfe_sec_emmc build_cfe_nand_tk build_cfe_emmc_tk build_cfe_nor build_cfe_sec_nor

else
dynamic_cfe:

endif

dynamic_cfe_clean: 
	-rm -rf cfe/build/broadcom/build_cfe*
	-rm  -f cfe/build/broadcom/bcm63xx_rom/*.S
	-rm  -f cfe/build/broadcom/bcm63xx_rom/*.cferamlz

clean_uboot:
	if [ -d bootloaders ]; then \
		$(MAKE) -C bootloaders distclean; \
	fi

ifeq ($(strip $(BRCM_FLASH_BUILD_NAND)),y)
build_cfe_nand: cfe/build/broadcom/build_cferom_nand/Makefile cfe/build/broadcom/build_cferam_nand/Makefile hosttools untar_cfe rdp_link
	$(MAKE) -C cfe/build/broadcom/build_cferom_nand/ RAM_BUILD=../build_cferam_nand BLD_NAND=1 BRCM_CHIP=$(BRCM_CHIP) PROFILE_FILE=$(PROFILE_FILE)
else
build_cfe_nand:
	@echo "no CFE for nand needed"
endif

ifeq ($(strip $(BUILD_HND_EAP_UBOOT)),y)
nandcfeimage dynamic_cfe: build_uboot
build_uboot:
	if [ ! -e bootloaders/u-boot-2017.09/.mailmap ]; then \
		$(MAKE) -C bootloaders uboot_prep; \
	fi
	$(MAKE) CROSS_COMPILE=$(KCROSS_COMPILE) -C bootloaders uboot_bcm9$(BRCM_CHIP)-nand
	$(MAKE) CROSS_COMPILE=$(KCROSS_COMPILE) -C bootloaders uboot_bcm9$(BRCM_CHIP)-nand-install
endif

ifeq ($(strip $(SECURE_BOOT_TURNKEY)),y)
build_cfe_nand_tk: cfe/build/broadcom/build_cferom_nand_tk/Makefile cfe/build/broadcom/build_cferam_nand_tk/Makefile hosttools untar_cfe rdp_link
	$(MAKE) -C cfe/build/broadcom/build_cferom_nand_tk/ RAM_BUILD=../build_cferam_nand_tk BLD_NAND=1 BRCM_CHIP=$(BRCM_CHIP) PROFILE_FILE=$(PROFILE_FILE) BLD_SEC_TK=$(BLD_SEC_TK)
ifeq ($(strip $(BUILD_EMMC_IMG)),y)
build_cfe_emmc_tk: cfe/build/broadcom/build_cferom_emmc_tk/Makefile cfe/build/broadcom/build_cferam_emmc_tk/Makefile hosttools untar_cfe rdp_link
	$(MAKE) -C cfe/build/broadcom/build_cferom_emmc_tk/ RAM_BUILD=../build_cferam_emmc_tk BLD_EMMC=1 BRCM_CHIP=$(BRCM_CHIP) PROFILE_FILE=$(PROFILE_FILE) BLD_SEC_TK=$(BLD_SEC_TK)
else
build_cfe_emmc_tk:
endif
else
build_cfe_nand_tk:
build_cfe_emmc_tk:
endif

ifeq ($(strip $(BUILD_EMMC_IMG)),y)
build_cfe_emmc: cfe/build/broadcom/build_cferom_emmc/Makefile cfe/build/broadcom/build_cferam_emmc/Makefile hosttools untar_cfe rdp_link
	$(MAKE) -C cfe/build/broadcom/build_cferom_emmc/ RAM_BUILD=../build_cferam_emmc BLD_EMMC=1 BRCM_CHIP=$(BRCM_CHIP) PROFILE_FILE=$(PROFILE_FILE)
else
build_cfe_emmc:
	@echo "no CFE for emmc needed"
endif

ifeq ($(strip $(BRCM_FLASH_BUILD_NOR)),y)
build_cfe_nor: cfe/build/broadcom/build_cferom_nor/Makefile cfe/build/broadcom/build_cferam_nor/Makefile hosttools untar_cfe rdp_link
ifneq ($(findstring _$(strip $(BTRM_BOOT_ONLY))_$(strip $(BRCM_KERNEL_ROOTFS))_$(strip $(BRCM_CHIP))_,_y_squashfs_63178_y_squashfs_47622_),)
	$(MAKE) -C cfe/build/broadcom/build_cferom_nor/ RAM_BUILD=../build_cferam_nor BRCM_CHIP=$(BRCM_CHIP) PROFILE_FILE=$(PROFILE_FILE) ROM_BUILD=../build_cferom_nor DISABLE_NOR_RAW_PARTITION=${DISABLE_NOR_RAW_PARTITION} CFG_SPLIT_NOR_CFE_ROM_RAM=1
else
	$(MAKE) -C cfe/build/broadcom/build_cferom_nor/ RAM_BUILD=../build_cferam_nor BRCM_CHIP=$(BRCM_CHIP) PROFILE_FILE=$(PROFILE_FILE) ROM_BUILD=../build_cferom_nor DISABLE_NOR_RAW_PARTITION=${DISABLE_NOR_RAW_PARTITION}
endif
else
build_cfe_nor:
	@echo "no CFE for nor needed"
endif

build_cfe_sec_nor: cfe/build/broadcom/build_cferom_sec_nor/Makefile cfe/build/broadcom/build_cferam_sec_nor/Makefile hosttools untar_cfe

build_cfe_sec_nand: cfe/build/broadcom/build_cferom_sec_nand/Makefile cfe/build/broadcom/build_cferam_sec_nand/Makefile hosttools untar_cfe

build_cfe_sec_emmc: cfe/build/broadcom/build_cferom_sec_emmc/Makefile cfe/build/broadcom/build_cferam_sec_emmc/Makefile hosttools untar_cfe

untar_cfe: cfe/cfe/api
	cd cfe && tar xfzk cfe*.tar.gz 2> /dev/null || true

.PHONY: untar_cfe cfe/cfe/api

cfe/build/broadcom/build_cferom_%/Makefile : cfe/build/broadcom/bcm63xx_rom/Makefile
	mkdir -p ${@D}
	cat $< >  $@

cfe/build/broadcom/build_cferam_%/Makefile : cfe/build/broadcom/bcm63xx_ram/Makefile
	mkdir -p ${@D}
	cat $< >  $@

.PHONY: dynamic_cfe dynamic_cfe_clean build_cfe_nand build_cfe_nand_tk build_cfe_emmc_tk build_cfe_emmc build_cfe_sec_nand build_cfe_sec_emmc build_uboot clean_uboot


ifeq ($(FB),y)
buildimage: $(DTBS) libcreduction gen_credits
else
buildimage: dynamic_cfe kernelbuild $(DTBS) libcreduction gen_credits linux_tools
endif
ifneq ($(strip $(BRCM_PERMIT_STDCPP)),)
	@if ls $(PROFILE_DIR)/fs.install/lib/libstdc* 2> /dev/null ; then  \
		echo -e "libstdc++ must be replaced with STLPORT";         \
		echo -e "override with BRCM_PERMIT_STDCPP=1 if ok";         \
		false;                                                     \
	fi
endif
ifeq ($(BUILD_DISABLE_EXEC_STACK),y)
ifneq ($(execstack_exec),)
	@echo no need to build execstack $(execstack_exec)
else
	make -C $(HOSTTOOLS_DIR) build_execstack;
endif
endif
	cd $(TARGETS_DIR); ./buildFS;

################
# BEEP related #
################	
ifneq ($(and $(strip $(BUILD_MODSW_EE)),$(strip $(BUILD_BRCM_CMS))),)
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmbeep.mk buildFS_BEE
endif
ifneq ($(BUILD_MODSW_EXAMPLEEE),)
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmbeep.mk buildFS_EXAMPLEEE
endif
ifneq ($(BUILD_MODSW_EE),)
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmbeep.mk buildFS_BEEPPREINSTALL
	cd $(TARGETS_DIR); ./buildFS;	
endif
ifneq ($(BUILD_MODSW_OPENWRTEE),)
	$(MAKE) -C $(BUILD_DIR)/build -f Bcmbeep.mk buildFS_OPENWRTEE
endif


ifeq ($(strip $(BRCM_RAMDISK_BOOT_EN)),y)
	cd $(TARGETS_DIR); ./buildFS_RD
endif
	cd $(TARGETS_DIR); \
		export CFE_RAM_FILE CFE_RAM_EMMC_FILE  ; \
		./buildFS2


	@mkdir -p $(IMAGES_DIR)
	$(MAKE) -f build/Makefile buildimage_final


shell:
	@echo "You are in a shell that includes the Makefile environment.  "exit" to return to normal"
	PS1='!_' bash --norc --noprofile

buildimage_final:
ifeq ($(strip $(BRCM_KERNEL_ROOTFS)),all)

###############
# NAND IMAGES #
###############
ifeq ($(strip $(BTRM_BOOT_ONLY)),y)
	@echo -e "No XIP to flash capability. Bootrom boot only. Build unsecure bootrom boot"

###################################################
# NAND UBI and JFFS2 UNSECURE BOOTROM BOOT IMAGES #
###################################################
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_16KB)),y)
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_16KB) --ubifs --bootfs bootfs16kb.img --rootfs ubi_rootfs16kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_16)_ubi --fsonly $(FLASH_NAND_FS_IMAGE_NAME_16)_ubi --unsecurehdr $(PRE_CFE_ROM)
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_16KB) --squbifs --bootfs bootfs16kb.img --rootfs squbi_rootfs16kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_16)_squbi --fsonly $(FLASH_NAND_FS_IMAGE_NAME_16)_squbi --unsecurehdr $(PRE_CFE_ROM)
endif
	$(foreach bnib,$(BUILD_NAND_IMG_BLKSIZE), $(call build_images_unsecure,B_$(BRCM_FLASH_NAND_LAYOUT_SPLIT)_L_$(BRCM_FLASH_NAND_LAYOUT_PUREUBI),$(CFE_ROM_FILE),$(bnib),$(FLASH_BOOT_OFS),$(PRE_CFE_ROM),--unsecurehdr,u$(BRCM_FLASH_NAND_ROOTFS_UBIFS),s$(BRCM_FLASH_NAND_ROOTFS_SQUBI))) 

else

######################################
# NAND UBI and JFFS2 XIP BOOT IMAGES #
######################################
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_16KB)),y)
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_16KB) --ubifs --bootfs bootfs16kb.img --rootfs ubi_rootfs16kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_16)_ubi --fsonly $(FLASH_NAND_FS_IMAGE_NAME_16)_ubi $(PRE_CFE_ROM)
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_16KB) --squbifs --bootfs bootfs16kb.img --rootfs squbi_rootfs16kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_16)_squbi --fsonly $(FLASH_NAND_FS_IMAGE_NAME_16)_squbi $(PRE_CFE_ROM)
endif
	$(foreach bnib,$(BUILD_NAND_IMG_BLKSIZE), $(call build_images_unsecure, B_$(BRCM_FLASH_NAND_LAYOUT_SPLIT)_L_$(BRCM_FLASH_NAND_LAYOUT_PUREUBI),$(CFE_ROM_FILE),$(bnib),$(FLASH_BOOT_OFS),$(PRE_CFE_ROM),,u$(BRCM_FLASH_NAND_ROOTFS_UBIFS),s$(BRCM_FLASH_NAND_ROOTFS_SQUBI)))


endif


#########################################
# NAND UBI and JFFS2 SECURE BOOT IMAGES #
#########################################
ifeq ($(strip $(BUILD_SECURE_BOOT)),y)
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_16KB)),y)
ifeq ($(strip $(BRCM_CHIP)),63268)
        # NOTE: 63268 small page nand bootsize is 128KB on purpose (ie $(FLASH_NAND_BLOCK_128KB)).  do not change
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_128KB) --ubifs --bootfs bootfs16kb_secureboot.img --rootfs ubi_rootfs16kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_16)_ubi_secureboot --fsonly $(FLASH_NAND_FS_IMAGE_NAME_16)_ubi_secureboot $(SECUREHDR)
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_128KB) --squbifs --bootfs bootfs16kb_secureboot.img --rootfs squbi_rootfs16kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_16)_squbi_secureboot --fsonly $(FLASH_NAND_FS_IMAGE_NAME_16)_squbi_secureboot $(SECUREHDR)
else
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --cfesecrom $(CFESEC_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_16KB) --ubifs --bootfs bootfs16kb_secureboot.img --rootfs ubi_rootfs16kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_16)_ubi_secureboot --fsonly $(FLASH_NAND_FS_IMAGE_NAME_16)_ubi_secureboot $(SECUREHDR)
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --cfesecrom $(CFESEC_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_16KB) --squbifs --bootfs bootfs16kb_secureboot.img --rootfs squbi_rootfs16kb.img --image $(FLASH_NAND_CFEROM_FS_IMAGE_NAME_16)_squbi_secureboot --fsonly $(FLASH_NAND_FS_IMAGE_NAME_16)_squbi_secureboot $(SECUREHDR)
endif
endif
	$(foreach bnib,$(BUILD_NAND_IMG_BLKSIZE), $(call build_images_secure, B_$(BRCM_FLASH_NAND_LAYOUT_SPLIT)_L_$(BRCM_FLASH_NAND_LAYOUT_PUREUBI),$(if $(SECURE_BOOT_TURNKEY),$(CFE_ROM_FILE_TK),$(CFE_ROM_FILE)),$(bnib),$(FLASH_BOOT_OFS),$(SECUREHDR),u$(BRCM_FLASH_NAND_ROOTFS_UBIFS),s$(BRCM_FLASH_NAND_ROOTFS_SQUBI),$(CFESEC_ROM_FILE)))
endif

    ifeq ($(strip $(SKIP_TIMESTAMP_IMAGE)),)
# copy images to images directory and add a timestamp
	find $(PROFILE_DIR) -name *_nand_cferom_*.w  -printf "%f\n" | while read name; do cp $(PROFILE_DIR)/$$name $(IMAGES_DIR)/$${name/.w/_$(BRCM_RELEASETAG)-$(shell date '+%y%m%d_%H%M').w}; done
    endif

endif

######################
# EMMC IMAGES        #
######################
ifneq ($(findstring _$(strip $(BRCM_KERNEL_ROOTFS))_,_all_),)
ifeq ($(strip $(BUILD_EMMC_IMG)),y)
#########################################
# EMMC UNSECURE BOOT EXT4/Squash IMAGES #
#########################################
ifeq ($(strip $(BLD_EMMC_BTROM_BOOT_IMAGES)),y)
	cp -vf $(CFE_RAM_EMMC_FILE) $(TARGET_BOOTFS)/cferam.000
	cd $(TARGETS_DIR); ./buildFS_EMMC $(TARGET_BOOTFS)
	if [ -f $(PROFILE_DIR)/filestruct_full_emmc.bin ]; then \
		$(BUILD_SBI_UNSEC) --in $(CFE_ROM_EMMC_FILE) --out $(PROFILE_DIR)/.tmpimage; \
		cd $(PROFILE_DIR); \
		echo "Creating fs_kernel eMMC images"; \
		$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(FS_KERNEL_IMAGE_NAME)_emmc_ext4 --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize 2048 --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/.tmpimage --rootfsfile $(PROFILE_DIR)/rootfs.ext4 --bootfsfile $(PROFILE_DIR)/filestruct_full_emmc.bin --mdatafile $(PROFILE_DIR)/metadata.bin ; \
		$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(FS_KERNEL_IMAGE_NAME)_emmc_squashfs --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize 2048 --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/.tmpimage --rootfsfile $(PROFILE_DIR)/rootfs.img --bootfsfile $(PROFILE_DIR)/filestruct_full_emmc.bin --mdatafile $(PROFILE_DIR)/metadata.bin ; \
		echo "Creating cfe_fs_kernel eMMC images"; \
		$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(CFE_FS_KERNEL_IMAGE_NAME)_emmc_ext4 --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize 2048 --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/.tmpimage --rootfsfile $(PROFILE_DIR)/rootfs.ext4 --bootfsfile $(PROFILE_DIR)/filestruct_full_emmc.bin --mdatafile $(PROFILE_DIR)/metadata.bin --include-cfe $(IMG_DDR_TYPE_OPT); \
		$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(CFE_FS_KERNEL_IMAGE_NAME)_emmc_squashfs --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize 2048 --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/.tmpimage --rootfsfile $(PROFILE_DIR)/rootfs.img --bootfsfile $(PROFILE_DIR)/filestruct_full_emmc.bin --mdatafile $(PROFILE_DIR)/metadata.bin --include-cfe $(IMG_DDR_TYPE_OPT); \
		echo "Generating default NVRAM binary"; \
		$(HOSTTOOLS_DIR)/createimg.pl --set  boardid=$(BRCM_BOARD_ID) voiceboardid=$(BRCM_VOICE_BOARD_ID) numbermac=$(BRCM_NUM_MAC_ADDRESSES) macaddr=$(BRCM_BASE_MAC_ADDRESS) tp=$(BRCM_MAIN_TP_NUM) psisize=$(BRCM_PSI_SIZE) logsize=$(BRCM_LOG_SECTION_SIZE) auxfsprcnt=$(BRCM_AUXFS_PERCENT) gponsn=$(BRCM_GPON_SERIAL_NUMBER) gponpw=$(BRCM_GPON_PASSWORD) --nvramfile $(HOSTTOOLS_DIR)/nvram.h --nvramdefsfile $(HOSTTOOLS_DIR)/nvram_defaults.h --config=$(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf --outputfile=$(BRCM_BOARD_ID)_nvram.bin --output_nvram_bin_only; \
		echo "Generating eMMC raw whole partition images"; \
		$(HOSTTOOLS_DIR)/create_emmc_rawimg.py --rootfs_file $(PROFILE_DIR)/rootfs.ext4 --bootfs_file $(PROFILE_DIR)/filestruct_full_emmc.bin --mdata_file $(PROFILE_DIR)/metadata.bin --nvram_file $(PROFILE_DIR)/$(BRCM_BOARD_ID)_nvram.bin --data_sizeMB 10  --rawfullimg_file $(FLASH_IMAGE_NAME_EMMC_DATA_PHYSPART)_ext4.w --cferom_file $(PROFILE_DIR)/.tmpimage --cferom_offsetkB 64 --rawcfeimg_file $(FLASH_IMAGE_NAME_EMMC_BOOT_PHYSPART).w --emmcdefs_file $(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)/emmc_base_defs.h; \
		$(HOSTTOOLS_DIR)/create_emmc_rawimg.py --rootfs_file $(PROFILE_DIR)/rootfs.img --bootfs_file $(PROFILE_DIR)/filestruct_full_emmc.bin --mdata_file $(PROFILE_DIR)/metadata.bin --nvram_file $(PROFILE_DIR)/$(BRCM_BOARD_ID)_nvram.bin --data_sizeMB 10 --rawfullimg_file $(FLASH_IMAGE_NAME_EMMC_DATA_PHYSPART)_squash.w --emmcdefs_file $(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)/emmc_base_defs.h; \
		rm -vf $(PROFILE_DIR)/.tmpimage; \
	fi;
#######################################
# EMMC SECURE BOOT EXT4 IMAGES        #
#######################################
ifeq ($(strip $(BUILD_SECURE_BOOT)),y)	
	$(BUILD_SBI_NOHDR) --in $(CFESEC_RAM_EMMC_FILE) -out $(TARGET_BOOTFS)/secram.000
	echo -e "/secram.000" >> $(HOSTTOOLS_DIR)/nocomprlist
ifeq ($(strip $(SECURE_BOOT_ARCH)),GEN3)
	$(BUILD_SBI_NOHDR_MFG) --in $(CFESEC_RAM_EMMC_FILE) -out $(TARGET_BOOTFS)/secmfg.000
	echo -e "/secmfg.000" >> $(HOSTTOOLS_DIR)/nocomprlist
endif
	cp -vf $(CFESEC_RAM_EMMC_FILE) $(TARGET_BOOTFS)/cferam.000
	cd $(TARGETS_DIR); ./buildFS_EMMC $(TARGET_BOOTFS)

	if [ -f $(PROFILE_DIR)/filestruct_full_emmc.bin ]; then \
		$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_EMMC_FILE) --cfesecrom $(CFESEC_ROM_EMMC_FILE) --blocksize 2048 --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_128KB) --image $(PROFILE_DIR)/.tmpimage $(SECUREHDR) --mediatype emmc; \
		cd $(PROFILE_DIR); \
		echo "Creating Secure fs_kernel eMMC images"; \
		$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(FS_KERNEL_IMAGE_NAME)_emmc_ext4_secureboot --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize 2048  --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/.tmpimage.bin --rootfsfile $(PROFILE_DIR)/rootfs.ext4 --bootfsfile $(PROFILE_DIR)/filestruct_full_emmc.bin --mdatafile $(PROFILE_DIR)/metadata.bin ; \
		$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(FS_KERNEL_IMAGE_NAME)_emmc_squashfs_secureboot --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP))	--board $(BRCM_BOARD_ID) --blocksize 2048 --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/.tmpimage.bin --rootfsfile $(PROFILE_DIR)/rootfs.img --bootfsfile $(PROFILE_DIR)/filestruct_full_emmc.bin --mdatafile $(PROFILE_DIR)/metadata.bin ; \
		echo "Creating Secure cfe_fs_kernel eMMC images"; \
		$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(CFE_FS_KERNEL_IMAGE_NAME)_emmc_ext4_secureboot --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize 2048 --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/.tmpimage.bin --rootfsfile $(PROFILE_DIR)/rootfs.ext4 --bootfsfile $(PROFILE_DIR)/filestruct_full_emmc.bin --mdatafile $(PROFILE_DIR)/metadata.bin --include-cfe $(IMG_DDR_TYPE_OPT); \
		$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(CFE_FS_KERNEL_IMAGE_NAME)_emmc_squashfs_secureboot --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize 2048 --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/.tmpimage.bin --rootfsfile $(PROFILE_DIR)/rootfs.img --bootfsfile $(PROFILE_DIR)/filestruct_full_emmc.bin --mdatafile $(PROFILE_DIR)/metadata.bin --include-cfe $(IMG_DDR_TYPE_OPT); \
		echo "Generating default NVRAM binary"; \
		$(HOSTTOOLS_DIR)/createimg.pl --set  boardid=$(BRCM_BOARD_ID) voiceboardid=$(BRCM_VOICE_BOARD_ID) numbermac=$(BRCM_NUM_MAC_ADDRESSES) macaddr=$(BRCM_BASE_MAC_ADDRESS) tp=$(BRCM_MAIN_TP_NUM) psisize=$(BRCM_PSI_SIZE) logsize=$(BRCM_LOG_SECTION_SIZE) auxfsprcnt=$(BRCM_AUXFS_PERCENT) gponsn=$(BRCM_GPON_SERIAL_NUMBER) gponpw=$(BRCM_GPON_PASSWORD) --nvramfile $(HOSTTOOLS_DIR)/nvram.h --nvramdefsfile $(HOSTTOOLS_DIR)/nvram_defaults.h --config=$(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf --outputfile=$(BRCM_BOARD_ID)_nvram.bin --output_nvram_bin_only; \
		echo "Generating Secure eMMC raw whole partition images"; \
		$(HOSTTOOLS_DIR)/create_emmc_rawimg.py --rootfs_file $(PROFILE_DIR)/rootfs.ext4 --bootfs_file $(PROFILE_DIR)/filestruct_full_emmc.bin --mdata_file $(PROFILE_DIR)/metadata.bin --nvram_file $(PROFILE_DIR)/$(BRCM_BOARD_ID)_nvram.bin --data_sizeMB 10 --rawfullimg_file $(FLASH_IMAGE_NAME_EMMC_DATA_PHYSPART)_ext4_secureboot.w --cferom_file $(PROFILE_DIR)/.tmpimage.bin --cferom_offsetkB 0 --rawcfeimg_file $(FLASH_IMAGE_NAME_EMMC_BOOT_PHYSPART)_secureboot.w --emmcdefs_file $(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)/emmc_base_defs.h; \
		$(HOSTTOOLS_DIR)/create_emmc_rawimg.py --rootfs_file $(PROFILE_DIR)/rootfs.img --bootfs_file $(PROFILE_DIR)/filestruct_full_emmc.bin --mdata_file $(PROFILE_DIR)/metadata.bin --nvram_file $(PROFILE_DIR)/$(BRCM_BOARD_ID)_nvram.bin --data_sizeMB 10 --rawfullimg_file $(FLASH_IMAGE_NAME_EMMC_DATA_PHYSPART)_squash_secureboot.w --emmcdefs_file $(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)/emmc_base_defs.h; \
		rm -f $(PROFILE_DIR)/.tmpimage.bin; \
	fi;
endif
endif

endif
endif

##############################
# NOR FLASH SQUASHFS IMAGES  #
##############################
ifeq ($(strip $(DESKTOP_LINUX)),)
ifneq ($(findstring _$(strip $(BRCM_KERNEL_ROOTFS))_,_all_ _squashfs_),)
ifneq ("$(wildcard $(CFE_FILE))","")
	cd $(PROFILE_DIR); \
	cp $(KERNEL_DIR)/vmlinux . ; \
	$(KSTRIP) --remove-section=.note --remove-section=.comment vmlinux; \
	$(KOBJCOPY) -O binary vmlinux vmlinux.bin; \
	$(HOSTTOOLS_DIR)/cmplzma -k -2 -$(KERNEL_COMPRESSION) vmlinux vmlinux.bin vmlinux.lz; \
	$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(FS_KERNEL_IMAGE_NAME) --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize $(BRCM_FLASHBLK_SIZE) --image-version $(IMAGE_VERSION) --cfefile $(CFE_FILE) --rootfsfile rootfs.img --kernelfile vmlinux.lz --dtbfile $(DTB_FILE); \
	$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(CFE_FS_KERNEL_IMAGE_NAME) --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize $(BRCM_FLASHBLK_SIZE) --image-version $(IMAGE_VERSION) --cfefile $(CFE_FILE) --rootfsfile rootfs.img --kernelfile vmlinux.lz --dtbfile $(DTB_FILE) --include-cfe $(IMG_DDR_TYPE_OPT); \
	$(HOSTTOOLS_DIR)/createimg.pl --set  boardid=$(BRCM_BOARD_ID) voiceboardid=$(BRCM_VOICE_BOARD_ID) numbermac=$(BRCM_NUM_MAC_ADDRESSES) macaddr=$(BRCM_BASE_MAC_ADDRESS) tp=$(BRCM_MAIN_TP_NUM) psisize=$(BRCM_PSI_SIZE) logsize=$(BRCM_LOG_SECTION_SIZE) auxfsprcnt=$(BRCM_AUXFS_PERCENT) gponsn=$(BRCM_GPON_SERIAL_NUMBER) gponpw=$(BRCM_GPON_PASSWORD) --inputfile=$(CFE_FS_KERNEL_IMAGE_NAME) --outputfile=$(FLASH_IMAGE_NAME) --nvramfile $(HOSTTOOLS_DIR)/nvram.h --nvramdefsfile $(HOSTTOOLS_DIR)/nvram_defaults.h --config=$(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf; \
	$(HOSTTOOLS_DIR)/addvtoken --endian $(ARCH_ENDIAN) --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --flashtype NOR $(IMG_DDR_TYPE_OPT) $(FLASH_IMAGE_NAME) $(FLASH_IMAGE_NAME).w



ifneq ($(strip $(BTRM_BOOT_ONLY)),y)
ifeq ($(strip $(BUILD_SECURE_BOOT)),y)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63268_63381_63138_63148_),)
ifeq ($(strip $(BRCM_CHIP)),63268)
	cat $(PROFILE_DIR)/vmlinux_secureboot.lz $(PROFILE_DIR)/vmlinux_secureboot.sig > $(PROFILE_DIR)/vmlinux_secureboot.lz.sig;
else
	cat $(PROFILE_DIR)/vmlinux.lz $(PROFILE_DIR)/vmlinux.sig > $(PROFILE_DIR)/vmlinux_secureboot.lz.sig;
endif
	echo "----- BUILD SBI $(BUILD_SBI)"
	cd $(PROFILE_DIR); \
	$(BUILD_SBI) --sec_opt spi --in $(CFESEC_FILE) --in1 $(CFE_FILE) -out $(PROFILE_DIR)/$(BRCM_CHIP)bi_nor.bin --max_size $(SECURE_BOOT_NOR_BOOT_SIZE) ; \
	$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(FS_KERNEL_IMAGE_NAME)_secureboot --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize $(SECURE_BOOT_NOR_BOOT_SIZE) --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/$(BRCM_CHIP)bi_nor.bin --rootfsfile rootfs.img --kernelfile vmlinux_secureboot.lz.sig --dtbfile $(DTB_FILE); \
	$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(CFE_FS_KERNEL_IMAGE_NAME)_secureboot --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize  $(SECURE_BOOT_NOR_BOOT_SIZE) --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/$(BRCM_CHIP)bi_nor.bin --rootfsfile rootfs.img --kernelfile vmlinux_secureboot.lz.sig --include-cfe --dtbfile $(DTB_FILE); \
	rm -f $(PROFILE_DIR)$(BRCM_CHIP)bi_nor.bin vmlinux_secureboot.lz.sig; \
	$(HOSTTOOLS_DIR)/createimg.pl --set boardid=$(BRCM_BOARD_ID) voiceboardid=$(BRCM_VOICE_BOARD_ID) numbermac=$(BRCM_NUM_MAC_ADDRESSES) macaddr=$(BRCM_BASE_MAC_ADDRESS) tp=$(BRCM_MAIN_TP_NUM) psisize=$(BRCM_PSI_SIZE) logsize=$(BRCM_LOG_SECTION_SIZE) auxfsprcnt=$(BRCM_AUXFS_PERCENT) gponsn=$(BRCM_GPON_SERIAL_NUMBER) gponpw=$(BRCM_GPON_PASSWORD) --inputfile=$(CFE_FS_KERNEL_IMAGE_NAME)_secureboot --outputfile=$(FLASH_IMAGE_NAME)_secureboot --nvramfile $(HOSTTOOLS_DIR)/nvram.h --nvramdefsfile $(HOSTTOOLS_DIR)/nvram_defaults.h --config=$(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf; \
	$(HOSTTOOLS_DIR)/addvtoken --endian $(ARCH_ENDIAN) --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --flashtype NOR --btrm 1 $(FLASH_IMAGE_NAME)_secureboot $(FLASH_IMAGE_NAME)_secureboot.w
endif
endif
else
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63178_47622_),)
ifneq ($(findstring _$(strip $(BRCM_KERNEL_ROOTFS))_, _squashfs_),)
###################################################
#       NOR UNSECURE BOOTROM BOOT IMAGES          #
###################################################
	echo "----- BUILD NOR UNSECURE BOOTROM BOOT IMAGES"
	cd $(PROFILE_DIR); \
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_FILE) --bootofs 0 --bootsize $$(( $(BRCM_FLASHBLK_SIZE) * 1024 )) --blocksize 2048 --image $(PROFILE_DIR)/.tmpimage --unsecurehdr --mediatype nor; \
	$(HOSTTOOLS_DIR)/bcmImageBuilder $(BRCM_ENDIAN_FLAGS) --output $(CFE_FS_KERNEL_IMAGE_NAME)_bootromboot --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --board $(BRCM_BOARD_ID) --blocksize $(BRCM_FLASHBLK_SIZE) --image-version $(IMAGE_VERSION) --cfefile $(PROFILE_DIR)/.tmpimage.bin --rootfsfile rootfs.img --kernelfile vmlinux.lz --dtbfile $(DTB_FILE) --include-cfe $(IMG_DDR_TYPE_OPT); \
	$(HOSTTOOLS_DIR)/createimg.pl --set  boardid=$(BRCM_BOARD_ID) voiceboardid=$(BRCM_VOICE_BOARD_ID) numbermac=$(BRCM_NUM_MAC_ADDRESSES) macaddr=$(BRCM_BASE_MAC_ADDRESS) tp=$(BRCM_MAIN_TP_NUM) psisize=$(BRCM_PSI_SIZE) logsize=$(BRCM_LOG_SECTION_SIZE) auxfsprcnt=$(BRCM_AUXFS_PERCENT) gponsn=$(BRCM_GPON_SERIAL_NUMBER) gponpw=$(BRCM_GPON_PASSWORD) --inputfile=$(CFE_FS_KERNEL_IMAGE_NAME)_bootromboot --outputfile=$(FLASH_IMAGE_NAME)_bootromboot --nvramfile $(HOSTTOOLS_DIR)/nvram.h --nvramdefsfile $(HOSTTOOLS_DIR)/nvram_defaults.h --config=$(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf; \
	$(HOSTTOOLS_DIR)/addvtoken --endian $(ARCH_ENDIAN) --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --flashtype NOR $(IMG_DDR_TYPE_OPT) $(FLASH_IMAGE_NAME)_bootromboot $(FLASH_IMAGE_NAME)_bootromboot.w;\
	$(HOSTTOOLS_DIR)/addvtoken --endian $(ARCH_ENDIAN) --chip $(or $(TAG_OVERRIDE),$(BRCM_CHIP)) --flashtype NOR $(IMG_DDR_TYPE_OPT) $(PROFILE_DIR)/.tmpimage.bin bcm9$(BRCM_CHIP)_cfe_bootromboot.w;\
	rm -f $(PROFILE_DIR)/.tmpimage.bin;\
	rm -f $(PROFILE_DIR)/$(CFE_FS_KERNEL_IMAGE_NAME);\
	rm -f $(PROFILE_DIR)/$(FLASH_IMAGE_NAME);\
	rm -f $(PROFILE_DIR)/$(FLASH_IMAGE_NAME).w; 
endif
endif
endif

ifeq ($(strip $(SKIP_TIMESTAMP_IMAGE)),)
ifeq ($(strip $(BTRM_BOOT_ONLY)),y)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63178_47622_),)
	@cp $(PROFILE_DIR)/$(FLASH_IMAGE_NAME)_bootromboot.w $(IMAGES_DIR)/$(FLASH_IMAGE_NAME)_bootromboot_$(BRCM_RELEASETAG)-$(shell date '+%y%m%d_%H%M').w
	@cp $(PROFILE_DIR)/$(CFE_FS_KERNEL_IMAGE_NAME)_bootromboot $(IMAGES_DIR)/$(CFE_FS_KERNEL_IMAGE_NAME)_bootromboot_$(BRCM_RELEASETAG)-$(shell date '+%y%m%d_%H%M')
else
	@cp $(PROFILE_DIR)/$(FLASH_IMAGE_NAME).w $(IMAGES_DIR)/$(FLASH_IMAGE_NAME)_$(BRCM_RELEASETAG)-$(shell date '+%y%m%d_%H%M').w
	@cp $(PROFILE_DIR)/$(CFE_FS_KERNEL_IMAGE_NAME) $(IMAGES_DIR)/$(CFE_FS_KERNEL_IMAGE_NAME)_$(BRCM_RELEASETAG)-$(shell date '+%y%m%d_%H%M')
endif
else
	@cp $(PROFILE_DIR)/$(FLASH_IMAGE_NAME).w $(IMAGES_DIR)/$(FLASH_IMAGE_NAME)_$(BRCM_RELEASETAG)-$(shell date '+%y%m%d_%H%M').w
	@cp $(PROFILE_DIR)/$(CFE_FS_KERNEL_IMAGE_NAME) $(IMAGES_DIR)/$(CFE_FS_KERNEL_IMAGE_NAME)_$(BRCM_RELEASETAG)-$(shell date '+%y%m%d_%H%M')
endif
endif

else
	@echo
	@echo -e "!!! NOR FLASH IMAGE BUILD SKIPPED !!!"
	@echo -e "To build NOR flash image:"
	@echo -e "   1) cd cfe/build/broadcom/bcm63xx_rom;make clean;make BRCM_CHIP=$(BRCM_CHIP)"
	@echo -e "   2) cp cfe$(BRCM_CHIP).bin $(TARGETS_DIR)/cfe"
	@echo -e "   3) build image again"
endif
	@echo
	@echo -e "Done! Image $(PROFILE) has been built in $(PROFILE_DIR)."
endif
else
	@echo "******************** SKIP NOR FLASH IMAGE BUILD for DESKTOP_LINUX ********************";
endif


########################
# eMMC CFEONLY IMAGES  #
########################
ifneq ($(if $(wildcard cfe/cfe/board),$(BUILD_DYNAMIC_CFE)),)
emmccfeimage: build_cfe_emmc
else
emmccfeimage: hosttools_nandcfe
endif
ifeq ($(strip $(BUILD_EMMC_IMG)),y)
ifeq ($(strip $(BLD_EMMC_BTROM_BOOT_IMAGES)),y)
	rm -rf  $(TARGET_FS)/
	mkdir -p $(TARGET_FS)
	echo "Generating NVRAM binary"; \
	$(HOSTTOOLS_DIR)/createimg.pl --set  boardid=$(BRCM_BOARD_ID) voiceboardid=$(BRCM_VOICE_BOARD_ID) numbermac=$(BRCM_NUM_MAC_ADDRESSES) macaddr=$(BRCM_BASE_MAC_ADDRESS) tp=$(BRCM_MAIN_TP_NUM) psisize=$(BRCM_PSI_SIZE) logsize=$(BRCM_LOG_SECTION_SIZE) auxfsprcnt=$(BRCM_AUXFS_PERCENT) gponsn=$(BRCM_GPON_SERIAL_NUMBER) gponpw=$(BRCM_GPON_PASSWORD) --nvramfile $(HOSTTOOLS_DIR)/nvram.h --nvramdefsfile $(HOSTTOOLS_DIR)/nvram_defaults.h --config=$(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf --outputfile=$(PROFILE_DIR)/$(BRCM_BOARD_ID)_nvram.bin --output_nvram_bin_only; 

	echo "Building Filestruct"; \
	cd $(TARGETS_DIR); ./buildFS_EMMC $(TARGET_BOOTFS) cfeonly 
	echo "Generating unsecure CFEROM"; \
	$(BUILD_SBI_UNSEC) --in $(CFE_ROM_EMMC_FILE) -out $(PROFILE_DIR)/.tmpimage; \
	echo "Generating unsecure raw images"; \
	$(HOSTTOOLS_DIR)/create_emmc_rawimg.py --bootfs_file $(PROFILE_DIR)/filestruct_full_emmc.bin --mdata_file $(PROFILE_DIR)/metadata.bin --nvram_file $(PROFILE_DIR)/$(BRCM_BOARD_ID)_nvram.bin --rawfullimg_file $(PROFILE_DIR)/$(FLASH_IMAGE_NAME_EMMC_DATA_PHYSPART)_cfeonly.w --cferom_file $(PROFILE_DIR)/.tmpimage --cferom_offsetkB 64 --rawcfeimg_file $(PROFILE_DIR)/$(FLASH_IMAGE_NAME_EMMC_BOOT_PHYSPART).w --emmcdefs_file $(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)/emmc_base_defs.h; 
ifeq ($(strip $(BUILD_SECURE_BOOT)),y)
	echo "Generating encrypted CFERAM"; \
	$(BUILD_SBI_NOHDR) --in $(CFESEC_RAM_EMMC_FILE) -out $(PROFILE_DIR)/fs/secram.000
	echo -e "/secram.000" >> $(HOSTTOOLS_DIR)/nocomprlist
ifeq ($(strip $(SECURE_BOOT_ARCH)),GEN3)
	$(BUILD_SBI_NOHDR_MFG) --in $(CFESEC_RAM_EMMC_FILE) -out $(PROFILE_DIR)/fs/secmfg.000
	echo -e "/secmfg.000" >> $(HOSTTOOLS_DIR)/nocomprlist
endif
	echo "Building Filestruct"; \
	cd $(TARGETS_DIR); ./buildFS_EMMC $(TARGET_BOOTFS) cfeonly 
	echo "Generating secure CFEROM"; \
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_EMMC_FILE) --cfesecrom $(CFESEC_ROM_EMMC_FILE) --blocksize 2048 --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_256KB) --image $(PROFILE_DIR)/.tmpimage $(SECUREHDR) --mediatype emmc; 
	echo "Generating secure raw images"; \
	$(HOSTTOOLS_DIR)/create_emmc_rawimg.py --bootfs_file $(PROFILE_DIR)/filestruct_full_emmc.bin --mdata_file $(PROFILE_DIR)/metadata.bin --nvram_file $(PROFILE_DIR)/$(BRCM_BOARD_ID)_nvram.bin --rawfullimg_file $(PROFILE_DIR)/$(FLASH_IMAGE_NAME_EMMC_DATA_PHYSPART)_cfeonly_secureboot.w --cferom_file $(PROFILE_DIR)/.tmpimage.bin --cferom_offsetkB 64 --rawcfeimg_file $(PROFILE_DIR)/$(FLASH_IMAGE_NAME_EMMC_BOOT_PHYSPART)_secureboot.w --emmcdefs_file $(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)/emmc_base_defs.h; 
endif
	rm -rf  $(TARGET_FS)
endif
endif

########################
# NAND CFEONLY IMAGES  #
########################
ifneq ($(if $(wildcard cfe/cfe/board),$(BUILD_DYNAMIC_CFE)),)
nandcfeimage: build_cfe_nand
else
nandcfeimage: hosttools_nandcfe
endif
	rm -rf  $(TARGET_FS)/
	mkdir -p $(TARGET_FS)
	cp -vf $(CFE_RAM_FILE) $(TARGET_FS)/cferam.000
	echo -e "/cferam.000" > $(HOSTTOOLS_DIR)/nocomprlist
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_16KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_16KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs16kb.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_128KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_128KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs128kb.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_256KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_256KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs256kb.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_512KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_512KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs512kb.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_1024KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_1024KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs1024kb.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif

ifeq ($(strip $(BUILD_SECURE_BOOT)),y)
	$(BUILD_SBI_NOHDR) --in $(CFESEC_RAM_FILE) -out $(PROFILE_DIR)/fs/secram.000
	echo -e "/secram.000" >> $(HOSTTOOLS_DIR)/nocomprlist
ifeq ($(strip $(SECURE_BOOT_ARCH)),GEN3)
	$(BUILD_SBI_NOHDR) --in $(CFESEC_RAM_FILE) -out $(PROFILE_DIR)/fs/secmfg.000
	echo -e "/secmfg.000" >> $(HOSTTOOLS_DIR)/nocomprlist
endif
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_16KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_16KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs16kb_secureboot.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_128KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_128KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs128kb_secureboot.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_256KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_256KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs256kb_secureboot.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_512KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_512KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs512kb_secureboot.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_1024KB)),y)
	$(HOSTTOOLS_DIR)/mkfs.jffs2 -v $(BRCM_ENDIAN_FLAGS) -p -n -e $(FLASH_NAND_BLOCK_1024KB) -r $(TARGET_FS) -o $(PROFILE_DIR)/rootfs1024kb_secureboot.img -N $(HOSTTOOLS_DIR)/nocomprlist
endif
endif
	rm $(HOSTTOOLS_DIR)/nocomprlist



ifeq ($(strip $(BTRM_BOOT_ONLY)),y)
	@echo -e "No XIP to flash capability. Bootrom boot only. Build unsecure bootrom boot"

ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_16KB)),y)
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_16KB) --rootfs rootfs16kb.img --image $(FLASH_BASE_IMAGE_NAME)_nand_cfeonly.16 --unsecurehdr $(PRE_CFE_ROM)
endif
	$(foreach bnib,$(BUILD_NAND_IMG_BLKSIZE), $(call build_cfeonly_images_unsecure,$(bnib),$(CFE_ROM_FILE),$(bnib),$(FLASH_BOOT_OFS),$(PRE_CFE_ROM),--unsecurehdr))

else

ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_16KB)),y)
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_16KB) --rootfs rootfs16kb.img --image $(FLASH_BASE_IMAGE_NAME)_nand_cfeonly.16 $(PRE_CFE_ROM)
endif

	$(foreach bnib,$(BUILD_NAND_IMG_BLKSIZE), $(call build_cfeonly_images_unsecure,$(BUILD_NAND_IMG_BLKSIZE_$(BLKSZ)KB),$(CFE_ROM_FILE),$(bnib),$(FLASH_BOOT_OFS),$(PRE_CFE_ROM)))

endif



ifeq ($(strip $(BUILD_SECURE_BOOT)),y)
ifeq ($(strip $(BUILD_NAND_IMG_BLKSIZE_16KB)),y)
ifeq ($(strip $(BRCM_CHIP)),63268)
	# NOTE: 63268 small page nand bootsize is 128K on purpose (ie $(FLASH_NAND_BLOCK_128KB)). Do not change.
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --cfesecrom $(CFESEC_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_128KB) --rootfs rootfs16kb_secureboot.img --image $(FLASH_BASE_IMAGE_NAME)_nand_cfeonly_secureboot.16 $(SECUREHDR) $(PRE_CFE_ROM)
else
	$(HOSTTOOLS_DIR)/scripts/bcmImageMaker --cferom $(CFE_ROM_FILE) --cfesecrom $(CFESEC_ROM_FILE) --blocksize $(FLASH_NAND_BLOCK_16KB) --bootofs $(FLASH_BOOT_OFS) --bootsize $(FLASH_NAND_BLOCK_16KB) --rootfs rootfs16kb_secureboot.img --image $(FLASH_BASE_IMAGE_NAME)_nand_cfeonly_secureboot.16 $(SECUREHDR) $(PRE_CFE_ROM)
endif
endif
	$(foreach bnib,$(BUILD_NAND_IMG_BLKSIZE), $(call build_cfeonly_images_secure,$(bnib),$(CFE_ROM_FILE),$(bnib),$(FLASH_BOOT_OFS),$(SECUREHDR),$(CFESEC_ROM_FILE)))
endif

	rm -f  $(TARGET_FS)/cferam.000
	rm -f  $(TARGET_FS)/secram.000
	rm -f  $(TARGET_FS)/secmfg.000

########################
#  BEEP package build  #
########################
-include $(BUILD_DIR)/make.beep

########################
# All CFEONLY IMAGES  #
########################
cfeimage: nandcfeimage emmccfeimage

###########################################
#
# System code clean-up
#
###########################################
CLEAN_WITH_SANITY_CHECK :=

.PHONY : clean clean1 bcmdrivers_clean data-model_clean clean_with_sanity_check 

clean:
	$(MAKE) -f build/Makefile -j1 BRCM_MAX_JOBS=1  clean1

clean1: bcmdrivers_clean data-model_clean dynamic_cfe_clean optee_clean \
	rdp_clean $(DTBS_CLEAN) clean_with_sanity_check clean_uboot
	rm -f $(HOSTTOOLS_DIR)/scripts/lxdialog/*.o
	rm -f .tmpconfig*
	-mv -f $(LAST_PROFILE_COOKIE) .check_clean
	rm -f $(LAST_PROFILE_COOKIE)
	rm -f $(HOST_PERLARCH_COOKIE)

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
	- CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk mrproper
	rm -f $(KERNEL_DIR)/arch/mips/defconfig
	rm -f $(KERNEL_DIR)/arch/arm/defconfig
	rm -f $(KERNEL_DIR)/arch/arm64/defconfig
	rm -f $(HOSTTOOLS_DIR)/lzma/decompress/*.o
	rm -f $(KERNEL_INCLUDE_LINK)
	rm -f $(KERNEL_MIPS_INCLUDE_LINK)
	rm -f $(KERNEL_ARM_INCLUDE_LINK)
	rm -f $(KERNEL_DIR)/.pre_kernelbuild
	rm -f $(KERNEL_DIR)/rdp_flags.txt
	rm -f $(BCM_SWVERSION_FILE)
ifeq ($(strip $(BCA_HNDROUTER)),)
	-find bcmdrivers/broadcom/net/wl/impl$(BCM_WLIMPL) -name build -type d -prune -exec rm -rf {} \; 2> /dev/null 
else
	rm -f bcmdrivers/broadcom/net/wl/impl$(BCM_WLIMPL)/Makefile
endif
ifneq ($(strip $(BUILD_LINUX_PERF)),)
	-$(MAKE) -C $(KERNEL_DIR)/tools/perf clean
endif

bcmdrivers_clean:
	-$(MAKE) -C bcmdrivers clean

CLEAN_WITH_SANITY_CHECK += userspace_clean
userspace_clean: sanity_check fssrc_clean 
	-rm -fr $(BCM_FSBUILD_DIR)
	-$(MAKE) -C userspace clean

data-model_clean:
ifeq ($(strip $(BUILD_BRCM_HNDROUTER_ALONE)),)
	-$(MAKE) -C data-model clean
else
	@true
endif

unittests_clean:
	-$(MAKE) -C unittests clean

CLEAN_WITH_SANITY_CHECK += target_clean
target_clean: sanity_check
	rm -f $(PROFILE_DIR)/*.img
	rm -f $(PROFILE_DIR)/*.bin
	rm -f $(PROFILE_DIR)/*.ini
	rm -f $(PROFILE_DIR)/rootfs*.ubifs
	rm -f $(PROFILE_DIR)/rootfs.ext4
	rm -f $(PROFILE_DIR)/vmlinux*
	rm -f $(PROFILE_DIR)/*.w
	rm -f $(PROFILE_DIR)/*.gz
	rm -f $(PROFILE_DIR)/*.srec
	rm -f $(PROFILE_DIR)/ramdisk
	rm -f $(PROFILE_DIR)/$(FS_KERNEL_IMAGE_NAME)*
	rm -f $(PROFILE_DIR)/$(CFE_FS_KERNEL_IMAGE_NAME)*
	rm -f $(PROFILE_DIR)/$(FLASH_IMAGE_NAME)*
	rm -fr $(PROFILE_DIR)/modules
	rm -fr $(PROFILE_DIR)/imagebuild/
	rm -fr $(PROFILE_DIR)/op
	rm -fr $(INSTALL_DIR)
	rm -fr $(BCM_FSBUILD_DIR)
	-find targets -name vmlinux -print -exec rm -f "{}" ";"
	rm -fr targets/TEMP
	rm -fr $(TARGET_FS)
	rm -f release/*credits.txt
ifeq ($(strip $(BRCM_KERNEL_ROOTFS)),all)
	rm -fr $(TARGET_BOOTFS)
endif

CLEAN_WITH_SANITY_CHECK += hosttools_clean	# for libcreduction clean
hosttools_clean:
	-$(MAKE) -C $(HOSTTOOLS_DIR) clean

.PHONY : hnd_dongle_clean
hnd_dongle_clean:
ifneq ($(strip $(BCA_HNDROUTER)),)
	# need to make sure soft link still exists
	-$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP) clean
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

version_info: sanity_check $(KERNEL_DIR)/.pre_kernelbuild
	@echo "$(MAKECMDGOALS):";\
	CURRENT_ARCH=$(KERNEL_ARCH) $(MAKE) -C $(BUILD_DIR)/build -f Bcmkernel.mk -j1 --silent version_info;
	# FIXME -- should not need -j1 here

###########################################
#
# System-wide exported variables
# (in alphabetical order)
#
###########################################

# EMBED_BALOO_BGN
export BUILD_BALOO BUILD_BALOOD_UDP BUILD_BALOO_UTIL
# EMBED_BALOO_END

export \
ACTUAL_MAX_JOBS            \
BRCMAPPS                   \
BRCM_BOARD                 \
BRCM_DRIVER_PCI            \
BRCM_EXTRAVERSION          \
BRCM_KERNEL_NETQOS         \
BRCM_KERNEL_ROOTFS         \
BRCM_KERNEL_AUXFS_JFFS2    \
BRCM_MIPS_ONLY_BUILD       \
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
BUILD_DNSPROBE             \
BUILD_DPROXY               \
BUILD_DNSSPOOF             \
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
BUILD_BCMSHARED            \
BUILD_MKSQUASHFS           \
BUILD_NAS                  \
BUILD_NVRAM                \
BUILD_PORT_MIRRORING       \
BUILD_PPPD                 \
PPP_AUTODISCONN            \
BUILD_SES                  \
BUILD_SIPROXD              \
BUILD_SLACTEST             \
BUILD_SNMP                 \
BUILD_SNTP                 \
BUILD_SOAP                 \
BUILD_SOAP_VER             \
BUILD_SSHD                 \
BUILD_SSHD_MIPS_GENKEY     \
BUILD_TOD                  \
BUILD_BRCM_CMS             \
BUILD_TR64                 \
BUILD_TR64_DEVICECONFIG    \
BUILD_TR64_DEVICEINFO      \
BUILD_TR64_LANCONFIGSECURITY \
BUILD_TR64_LANETHINTERFACECONFIG \
BUILD_TR64_LANHOSTS        \
BUILD_TR64_LANHOSTCONFIGMGMT \
BUILD_TR64_LANUSBINTERFACECONFIG \
BUILD_TR64_LAYER3          \
BUILD_TR64_MANAGEMENTSERVER  \
BUILD_TR64_TIME            \
BUILD_TR64_USERINTERFACE   \
BUILD_TR64_QUEUEMANAGEMENT \
BUILD_TR64_LAYER2BRIDGE   \
BUILD_TR64_WANCABLELINKCONFIG \
BUILD_TR64_WANCOMMONINTERFACE \
BUILD_TR64_WANDSLINTERFACE \
BUILD_TR64_WANDSLLINKCONFIG \
BUILD_TR64_WANDSLCONNECTIONMGMT \
BUILD_TR64_WANDSLDIAGNOSTICS \
BUILD_TR64_WANETHERNETCONFIG \
BUILD_TR64_WANETHERNETLINKCONFIG \
BUILD_TR64_WANIPCONNECTION \
BUILD_TR64_WANPOTSLINKCONFIG \
BUILD_TR64_WANPPPCONNECTION \
BUILD_TR64_WLANCONFIG      \
BUILD_TR69C                \
BUILD_TR69_QUEUED_TRANSFERS \
BUILD_TR69C_SSL            \
BUILD_TR69_XBRCM           \
BUILD_TR69_UPLOAD          \
BUILD_TR69C_VENDOR_RPC     \
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
BUILD_TR69C_BCM_SSL        \
BUILD_IPV6                 \
BUILD_BOARD_LOG_SECTION    \
BRCM_LOG_SECTION_SIZE      \
BRCM_FLASHBLK_SIZE         \
BRCM_AUXFS_PERCENT         \
BRCM_BACKUP_PSI            \
BUILD_IPSEC                \
BUILD_MoCACTL              \
BUILD_MoCACTL2             \
BUILD_6802_MOCA            \
BRCM_MOCA_AVS              \
BUILD_GPON                 \
BUILD_GPONCTL              \
BUILD_PMON                 \
BUILD_BUZZZ                \
BUILD_BOUNCE               \
BUILD_HELLO                \
BUILD_SPUCTL               \
RELEASE_BUILD              \
NO_PRINTK_AND_BUG          \
FLASH_NAND_BLOCK_16KB      \
FLASH_NAND_BLOCK_128KB     \
FLASH_NAND_BLOCK_256KB     \
FLASH_NAND_BLOCK_512KB     \
FLASH_NAND_BLOCK_1024KB     \
FLASH_NAND_BLOCK_2056KB     \
BRCM_CONFIG_HIGH_RES_TIMERS \
BRCM_SWITCH_SCHED_SP        \
BRCM_SWITCH_SCHED_WRR       \
BUILD_SWMDK                 \
BUILD_IQCTL                 \
BUILD_BPMCTL                \
BUILD_EPONCTL               \
BUILD_ETHTOOL               \
BUILD_TMS                   \
IMAGE_VERSION               \
TOOLCHAIN_PREFIX            \
PROFILE_KERNEL_VER          \
KERNEL_LINKS_DIR            \
LINUX_VER_STR               \
KERNEL_DIR                  \
FORCE                       \
BUILD_VLAN_AGGR             \
BUILD_DPI                   \
BUILD_MAP                   \
BRCM_1905_FM                \
BUILD_BRCM_CMS              \
BUILD_WEB_SOCKETS           \
BUILD_WEB_SOCKETS_TEST      \
BRCM_1905_TOPOLOGY_WEB_PAGE \
BUILD_DISABLE_EXEC_STACK    \
BUILD_DBUS                  \
BUILD_LXC                   \
NO_MINIFY                   \
BRCM_PARTITION_CFG_FILE     \
BCM_SPEEDYGET				

###########################################
# End of the real part
###########################################



