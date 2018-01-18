#
# Toplevel Makefile for the BCM947xx Linux Router release
#
# Copyright 2005, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.53 2005/04/25 03:54:37 tallest Exp $
#

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

# To rebuild everything and all configurations:
#  make distclean
#  make libc (usually doesn't need to be done ???)
#  make V1=whatever V2=sub-whatever VPN=vpn3.6 a b c d m
# The 1st "whatever" would be the build number, the sub-whatever would
#	be the update to the version.
#
# Example:
# make V1=8516 V2="-jffs.1" a b c d m s

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#
# To rebuild a part of code and generate a new firmware file:
#

# Example: rebuild rc (clean rc, compile rc, install rc to INSTALL directory, generate TARGET tree, generate Firmware file)
#  make rc-clean mk-rc rc-install gen_target image
#
# Note: the "rc-clean mk-rc" may skip if the dependency in Makefile is okay.
#

# Example: rebuild kernel and modules
#  make kernel gen_target image
#

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

PLATFORM_ROUTER:=

export HND_ROUTER := $(shell pwd | sed 's/.*hnd/y/g')
# declare the init var here. Don't include router's arch related var here due to it will break the multi-arch rules
ifeq ($(HND_ROUTER),y)
export BCM_CHIP := 4908
export HND_SRC := $(shell pwd | sed 's/\(.*src-rt-.*hnd\).*/\1/')
export SRCBASE := $(shell pwd | sed 's/\(.*src-rt-.*hnd\).*/\1\/bcmdrivers\/broadcom\/net\/wl\/bcm9$(BCM_CHIP)\/main\/src/')
export SRC_ROOT := $(HND_SRC)/../src
export BUSYBOX := busybox-1.24.1
export BUSYBOX_DIR := $(SRCBASE)/router/busybox-1.24.1/busybox-1.24.1
else
export SRCBASE := $(shell pwd)
ifeq ($(or $(BCM7),$(BCM_7114),$(BCM9)),y)
export SRC_ROOT := $(SRCBASE)/../../src
else
export SRC_ROOT := $(SRCBASE)/../src
endif
export BUSYBOX := busybox
export BUSYBOX_DIR := $(SRCBASE)/router/$(BUSYBOX)
endif
export SRCBASEDIR := $(shell pwd | sed 's/.*release\///g')
RELEASEDIR := $(shell (cd $(SRC_ROOT)/.. && pwd -P))
export PATH := $(RELEASEDIR)/tools:$(SRCBASE)/ctools:$(PATH)

# tmp depend on busybox
E2FSPROGS := $(if $(filter $(BUSYBOX),busybox-1.17.4),n,y)

ifeq ($(ASUSWRTTARGETMAKDIR),)
include ./target.mak
else
include $(ASUSWRTTARGETMAKDIR)/target.mak
endif

ifeq ($(ASUSWRTVERSIONCONFDIR),)
include ./version.conf
else
include $(ASUSWRTVERSIONCONFDIR)/version.conf
endif

export BRANCH := $(shell git branch)

ifneq ($(RCNO),)
RCSTRING="rc$(RCNO)"
RC_EXT_NO1=$(shell expr $(RCNO) + 1)
RC_EXT_NO=$(shell expr $(RC_EXT_NO1) \* 10000)

# At runtime, set the tag of RCNO.
#export RUN_TAG=$(shell git tag -fa asuswrt_$(KERNEL_VER).$(FS_VER).$(SERIALNO)$(RCSTRING) -m "Released $(RCSTRING)" HEAD)
else
RCSTRING=""
RC_EXT_NO=0
endif
EXTENDNO1=$(shell git log --pretty=oneline asuswrt_$(KERNEL_VER).$(FS_VER).$(SERIALNO)..HEAD | wc -l)

#ifeq ($(BRANCH),)
#include ./router/extendno.conf
#else
#ifneq ($(EXTENDTYPE),)
#EXSTRING="-$(EXTENDTYPE)"
#else
#EXSTRING=""
#endif
#export EXTENDNO := $(shell expr $(EXTENDNO1) + $(RC_EXT_NO))-g$(shell git log --pretty=format:'%h' -n 1|sed -e "s,\([0-9a-z]\{7\}\)[0-9a-z]*,\1,")$(EXSTRING)
#endif

ifeq ($(EXTENDNO),)
export EXTENDNO := 0-g$(shell git log --pretty=format:'%h' -n 1)
endif

export SWPJNAME := $(shell git branch | grep "*" | grep "swpj-" | sed 's/.*swpj-//g')

ifneq ($(SWPJNAME),)
export SWPJVERNO := $(shell cat $(SRCBASE)/router/APP-IPK/$(SWPJNAME)/CONTROL/control | grep "Version:" | sed 's/Version: //g')
export SWPJEXTENDNO := $(shell git describe --match swpj_$(SWPJNAME)_$(SWPJVERNO) | sed 's/swpj_$(SWPJNAME)_$(SWPJVERNO)-//g')
ifneq ($(SWPJVERNO),)
export SWPJVER := $(SWPJNAME)_$(SWPJVERNO)_
export SWPJEXTENDNO := $(shell git describe --match swpj_$(SWPJNAME)_$(SWPJVER) | sed 's/swpj_$(SWPJNAME)_$(SWPJVER)-//g')
endif

export SWPJEXTENDNO := $(shell git describe --match swpj_$(SWPJNAME)_$(SWPJVERNO) | sed 's/swpj_$(SWPJNAME)_$(SWPJVERNO)-//g')

ifneq ($(SWPJEXTENDNO),)
export EXTENDNO := $(SWPJEXTENDNO)
else
export EXTENDNO := 0-g$(shell git log --pretty=format:'%h' -n 1|sed -e "s,\([0-9a-z]\{7\}\)[0-9a-z]*,\1,")
endif
endif

ifeq ($(HND_ROUTER),y)
-include $(SRCBASE)/.config
export CONFIG_BCMWL5=y
export CONFIG_LINUX26=y
	# its multi-arch in 94908, specify ARCH in kbuild result bad
endif
ifneq ($(BUILDREV),)
export BUILDREV := -g$(shell git log --pretty=format:'%h' -n 1)
endif
-include .config
#
# include platform.mak after include .config
# some definitions in platform.mak may be defined as different value, if .config exist.
include ./platform.mak

-include ./dsl.mak

ifeq ($(HND_ROUTER),y)
export PROFILE ?= 94908HND
export BUILD_NAME ?= $(shell echo $(MAKECMDGOALS) | tr a-z A-Z)
include ./make.hndrt
export LINUXDIR := $(HND_SRC)/kernel/linux-4.1
export CROSS_COMPILER_PREFIX := arm-glibc-
export BCMEX7 := _arm_94908hnd
export BOARD_ID := $(shell cat $(PROFILE_PATH) | grep BRCM_BOARD_ID | awk -F '\"' '{print $$2}')
endif

ifneq ($(HND_ROUTER),y)		# the values have other usage in hnd models
ifneq ($(findstring -,$(PLATFORM))$(findstring src,$(KERNEL_BINARY))$(findstring src,$(LINUXDIR)),-srcsrc)
$(error Needs to define Platform-specific definitions in platform.mak)
endif
endif
export PLATFORMDIR := $(SRCBASE)/router/$(PLATFORM)

ifneq ($(BUILD_NAME),)
ifneq ($(BASE_MODEL),)
MODEL = $(subst -,,$(subst +,P,$(BASE_MODEL)))
else ifneq ($(findstring 4G-,$(BUILD_NAME)),)
MODEL = RT$(subst -,,$(BUILD_NAME))
else ifneq ($(findstring DSL,$(BUILD_NAME)),)
MODEL = $(subst -,_,$(BUILD_NAME))
else
MODEL = $(subst -,,$(subst +,P,$(BUILD_NAME)))
endif
export MODEL
export CFLAGS += -D$(MODEL)
export $(MODEL):=y
endif

EXTRA_KERNEL_YES_CONFIGS_1 := $(filter %=y %=Y,$(EXTRA_KERNEL_CONFIGS))
EXTRA_KERNEL_NO_CONFIGS_1 := $(filter %=n %=N,$(EXTRA_KERNEL_CONFIGS))
EXTRA_KERNEL_MOD_CONFIGS_1 := $(filter %=m %=M,$(EXTRA_KERNEL_CONFIGS))
EXTRA_KERNEL_VAL_CONFIGS := $(filter-out $(EXTRA_KERNEL_YES_CONFIGS_1) $(EXTRA_KERNEL_NO_CONFIGS_1) $(EXTRA_KERNEL_MOD_CONFIGS_1),$(EXTRA_KERNEL_CONFIGS))

EXTRA_KERNEL_YES_CONFIGS := $(subst =y,,$(subst =Y,,$(EXTRA_KERNEL_YES_CONFIGS_1)))
EXTRA_KERNEL_NO_CONFIGS := $(subst =n,,$(subst =N,,$(EXTRA_KERNEL_NO_CONFIGS_1)))
EXTRA_KERNEL_MOD_CONFIGS := $(subst =m,,$(subst =M,,$(EXTRA_KERNEL_MOD_CONFIGS_1)))

ifeq ($(NVRAM_SIZE),)
ifeq ($(NVRAM_64K),y)
NVRAM_SIZE=0x10000
else
NVRAM_SIZE=0x8000
endif
endif

CTAGS_EXCLUDE_OPT := --exclude=kernel_header --exclude=$(PLATFORM)
CTAGS_DEFAULT_DIRS := $(SRC_ROOT)/router/rc $(SRC_ROOT)/router/httpd $(SRC_ROOT)/router/shared $(SRC_ROOT)/router/www

uppercase_N = $(shell echo $(N) | tr a-z  A-Z)
lowercase_N = $(shell echo $(N) | tr A-Z a-z)
uppercase_B = $(shell echo $(BUILD_NAME) | tr a-z  A-Z)
lowercase_B = $(shell echo $(BUILD_NAME) | tr A-Z a-z)
BUILD_TIME := $(shell LC_ALL=C date -u)
BUILD_USER ?= $(shell whoami)
BUILD_INFO := $(shell git log --pretty="%h" -n 1|sed -e "s,\([0-9a-z]\{7\}\)[0-9a-z]*,\1,")

ifeq ($(CONFIG_LINUX26),y)
mips_rev = $(if $(filter $(MIPS32),r2),MIPSR2,MIPSR1)
KERN_SIZE_OPT ?= n
else
mips_rev =
KERN_SIZE_OPT ?= y
endif

ifeq ($(FAKEID),y)
export IMGNAME := $(BUILD_NAME)_$(FORCE_SN)_$(SWPJVER)$(FORCE_EN)$(BUILDREV)
else
export IMGNAME := $(BUILD_NAME)_$(SERIALNO)_$(SWPJVER)$(EXTENDNO)$(BUILDREV)
endif

ifeq ($(BUILD_NAME),DSL-AC68U)
TCFWVER := $(shell cat ./tc_fw/fwver.conf)
DSLIMGNAME=$(IMGNAME)_DSL_$(TCFWVER)
else ifeq ($(BUILD_NAME),DSL-N55U)
DSLIMGNAME=$(IMGNAME)_Annex_A
else ifeq ($(BUILD_NAME),DSL-N55U-B)
DSLIMGNAME=$(IMGNAME)_Annex_B
endif

ifeq ($(PBDIR),)
export PBDIR   := ../../../asuswrt.prebuilt/$(BUILD_NAME).$(KERNEL_VER).$(FS_VER).$(SERIALNO).$(EXTENDNO)
endif

# If platform specific software packages exist, PLATFORM_ROUTER should be defined in platform.mak
export PLATFORM_ROUTER

define bluecave_mkimage_extra_checks
$(if $(CONFIG_UBOOT_CONFIG_LTQ_IMAGE_EXTRA_CHECKS), \
	-B $(CONFIG_UBOOT_CONFIG_VENDOR_NAME) \
	-V $(CONFIG_UBOOT_CONFIG_BOARD_NAME) \
	-b $(CONFIG_UBOOT_CONFIG_BOARD_VERSION) \
	-c $(CONFIG_UBOOT_CONFIG_CHIP_NAME) \
	-p $(CONFIG_UBOOT_CONFIG_CHIP_VERSION) \
	-s $(CONFIG_UBOOT_CONFIG_SW_VERSION) \
)
endef

define build_bluecave_image
	mips-openwrt-linux-uclibc-objcopy -O binary -R .reginfo -R .notes -R .note -R .comment -R .mdebug -R .note.gnu.build-id -S linux/linux-3.10.104/vmlinux ./vmlinux
	mips-openwrt-linux-uclibc-objcopy -R .reginfo -R .notes -R .note -R .comment -R .mdebug -R .note.gnu.build-id -S linux/linux-3.10.104/vmlinux ./vmlinux.elf
	cp -fpR linux/linux-3.10.104/vmlinux ./vmlinux.debug
	cp ./vmlinux ./vmlinux-easy350_anywan_router_800m
	tools/dtc -O dtb -o ./easy350_anywan_router_800m.dtb ./proprietary/dts/easy350_anywan_router_800m.dts
	tools/host/bin/patch-dtb ./vmlinux-easy350_anywan_router_800m ./easy350_anywan_router_800m.dtb 32768
	tools/host/bin/lzma e ./vmlinux-easy350_anywan_router_800m ./vmlinux-easy350_anywan_router_800m.lzma
	mv ./vmlinux-easy350_anywan_router_800m.lzma ./vmlinux.lzma
	tools/u-boot-2010.06/tools/mkimage -A mips -O linux -T kernel -a 0x80002000 -C lzma -e 0x80002000 -n 'MIPS OpenWrt Linux-3.10.104' -d ./vmlinux.lzma ./uImage
	len2=`wc -c ./vmlinux.lzma | awk '{ printf $$1 }'` ; \
	echo "Raymond: $$len2"
	len=`wc -c ./vmlinux.lzma | awk '{ printf $$1 }'`; pad=`expr  131072 - $$len %  131072`; pad=`expr $$pad %  131072`; pad=`expr $$pad -  64`; [ $$pad -lt 0 ] && pad=0; echo pad is $$pad; echo len is $$len; cat ./vmlinux.lzma > ./vmlinux.lzma.padded; dd if=/dev/zero of=./vmlinux.lzma.padded bs=1 count=$$pad seek=$$len
	load_addr=0x$(shell grep -w _text linux/linux-3.10.104/System.map 2>/dev/null| awk '{ printf "%s", $$1 }'); \
	entry_addr=0x$(shell grep -w kernel_entry linux/linux-3.10.104/System.map 2>/dev/null| awk '{ printf "%s", $$1 }'); \
	tools/u-boot-2010.06/tools/mkimage -A mips -O linux -T kernel \
		-a $(s_load_addr) -C $(compression_type) -e $(s_entry_addr) \
	-n '$(image_header)' $(call bluecave_mkimage_extra_checks) \
	-d ./vmlinux.lzma.padded ./uImage.padded
	cp -f ./uImage.padded $(PLATFORMDIR)/uImage
	# ------ rootfs --------
	tools/host/bin/mksquashfs4 $(PLATFORMDIR)/target $(PLATFORMDIR)/root.squashfs -noappend -root-owned -comp xz -Xpreset 9 -Xe -Xlc 0 -Xlp 2 -Xpb 2 -processors 1
	tools/u-boot-2010.06/tools/mkimage -A MIPS -O Linux -C lzma -T filesystem -e 0x00 -a 0x00 -n "LTQCPE RootFS" \
	-d $(PLATFORMDIR)/root.squashfs $(PLATFORMDIR)/rootfs.img.padded
	cp -f $(PLATFORMDIR)/rootfs.img.padded $(PLATFORMDIR)/rootfs.img
	IMAGE_LIST="$(PLATFORMDIR)/rootfs.img \
		$(PLATFORMDIR)/uImage"; \
	ONEIMAGE="image/fullimage.img"; \
	PLATFORM="XRX500" ; \
	rm -f $$ONEIMAGE; \
	for i in $$IMAGE_LIST; do \
		if [ -e $$i  ] ; then \
			len=`wc -c $$i | awk '{ printf $$1 }'`; \
			pad=`expr 16 - $$len % 16`; \
			pad=`expr $$pad % 16`; \
			if [ -e $$ONEIMAGE.tmp ] ; then \
				cat $$i >> $$ONEIMAGE.tmp; \
			else \
				cat $$i > $$ONEIMAGE.tmp; \
			fi; \
			while [ $$pad -ne 0 ]; do \
				echo -n "0" >> $$ONEIMAGE.tmp; \
				pad=`expr $$pad - 1`; \
			done; \
		else \
			echo "$$i not found!"; \
			rm -f $$ONEIMAGE.tmp; \
			exit 1; \
		fi; \
	done; \
	tools/u-boot-2010.06/tools/mkimage -A MIPS -O Linux -C none -T multi -e 0x00 -a 0x00 \
		-n  \
		"$$PLATFORM Fullimage" -d $$ONEIMAGE.tmp $$ONEIMAGE; \
	rm -f $$ONEIMAGE.tmp; \
	chmod 644 $$ONEIMAGE;
	cp proprietary/gphy_firmware.img image/
	cp proprietary/uImage_bootcore image/
	cp proprietary/u-boot-nand.bin image/
	rm -f vmlinux*
	rm -f uImage*
	rm -f easy350_anywan_router_800m.dtb
	dd if=/dev/zero count=1 bs=128 | tr "\000" "\145" > ./eof.txt
	rm -f image/tmp-linux.trx; ctools/trx -o image/tmp-linux.trx image/fullimage.img ./eof.txt
	ctools/trx_asus -i image/tmp-linux.trx -r $(BUILD_NAME),$(KERNEL_VER).$(FS_VER),$(SERIALNO),$(EXTENDNO),image/$(IMGNAME).trx
	rm -f ./eof.txt
endef

default:
	@if [ -f .config -a "$(BUILD_NAME)" != "" ] ; then \
		$(MAKE) bin ; \
	else \
		echo "Source tree is not configured.  Run make with model name." ; \
	fi

ifeq ($(HND_ROUTER),y)
cfe_sysdep:
	if [ -d $(SRCBASE)/../../../../../../../targets/cfe/sysdeps/$(BUILD_NAME) ]; then \
		cp -f $(SRCBASE)/../../../../../../../targets/cfe/sysdeps/$(BUILD_NAME)/cfe4908.bin $(SRCBASE)/../../../../../../../targets/cfe/. ; \
		cp -f $(SRCBASE)/../../../../../../../targets/cfe/sysdeps/$(BUILD_NAME)/cfe4908ram.bin $(SRCBASE)/../../../../../../../targets/cfe/. ; \
		cp -f $(SRCBASE)/../../../../../../../targets/cfe/sysdeps/$(BUILD_NAME)/cfe4908rom.bin $(SRCBASE)/../../../../../../../targets/cfe/. ; \
	else \
		cp -f $(SRCBASE)/../../../../../../../targets/cfe/sysdeps/default/cfe4908.bin $(SRCBASE)/../../../../../../../targets/cfe/. ; \
		cp -f $(SRCBASE)/../../../../../../../targets/cfe/sysdeps/default/cfe4908ram.bin $(SRCBASE)/../../../../../../../targets/cfe/. ; \
		cp -f $(SRCBASE)/../../../../../../../targets/cfe/sysdeps/default/cfe4908rom.bin $(SRCBASE)/../../../../../../../targets/cfe/. ; \
	fi;

wl_sysdep:
	if [ -d $(SRCBASE)/../../../impl51/4365/src/wl/sysdeps/$(BUILD_NAME) ]; then \
		if [ "$(HND_MFG)" = "y" ] && [ -f $(SRCBASE)/../../../impl51/4365/src/wl/sysdeps/$(BUILD_NAME)/clm/src/wlc_clm_data_mfg.c ]; then \
			cp -f $(SRCBASE)/../../../impl51/4365/src/wl/sysdeps/$(BUILD_NAME)/clm/src/wlc_clm_data_mfg.c $(SRCBASE)/../../../impl51/4365/src/wl/clm/src/wlc_clm_data.c ; \
		elif [ -d $(SRCBASE)/../../../impl51/4365/src/wl/sysdeps/$(BUILD_NAME)/clm ]; then \
			cp -f $(SRCBASE)/../../../impl51/4365/src/wl/sysdeps/$(BUILD_NAME)/clm/src/wlc_clm_data.c $(SRCBASE)/../../../impl51/4365/src/wl/clm/src/. ; \
		fi; \
		if [ -d $(SRCBASE)/../../../impl51/4365/src/wl/sysdeps/$(BUILD_NAME)/clm/types ]; then \
			cp -f $(SRCBASE)/../../../impl51/4365/src/wl/sysdeps/$(BUILD_NAME)/clm/types/*_access.clm $(SRCBASE)/../../../impl51/4365/src/wl/clm/types/. ; \
		fi; \
	else \
		if [ -d $(SRCBASE)/../../../impl51/4365/src/wl/sysdeps/default/clm ]; then \
			cp -f $(SRCBASE)/../../../impl51/4365/src/wl/sysdeps/default/clm/src/wlc_clm_data.c $(SRCBASE)/../../../impl51/4365/src/wl/clm/src/. ; \
		fi; \
	fi;
	if [ -d $(SRCBASE)/../../../impl51/dhd/src/shared/sysdeps/$(BUILD_NAME) ]; then \
		if [ "$(HND_MFG)" = "y" ] && [ -f $(SRCBASE)/../../../impl51/dhd/src/shared/sysdeps/$(BUILD_NAME)/rtecdc_4366c0_mfg.h ]; then \
			cp -f $(SRCBASE)/../../../impl51/dhd/src/shared/sysdeps/$(BUILD_NAME)/rtecdc_4366c0_mfg.h $(SRCBASE)/../../../impl51/dhd/src/shared/rtecdc_4366c0.h ; \
		elif [ -f $(SRCBASE)/../../../impl51/dhd/src/shared/sysdeps/$(BUILD_NAME)/rtecdc_4366c0.h ]; then \
			cp -f $(SRCBASE)/../../../impl51/dhd/src/shared/sysdeps/$(BUILD_NAME)/rtecdc_4366c0.h $(SRCBASE)/../../../impl51/dhd/src/shared/. ; \
		fi; \
	else \
		if [ -f $(SRCBASE)/../../../impl51/dhd/src/shared/sysdeps/default/rtecdc_4366c0.h ]; then \
			cp -f $(SRCBASE)/../../../impl51/dhd/src/shared/sysdeps/default/rtecdc_4366c0.h $(SRCBASE)/../../../impl51/dhd/src/shared/. ; \
		fi; \
	fi;
endif

rt_ver:
	echo "make rt_ver "
	@echo '#ifndef RTVERSION' > router/shared/version.h
	@echo '#define RT_MAJOR_VERSION "$(KERNEL_VER)"' >> router/shared/version.h
	@echo '#define RT_MINOR_VERSION "$(FS_VER)"' >> router/shared/version.h
	@echo '#define RT_VERSION "$(KERNEL_VER).$(FS_VER)"' >> router/shared/version.h
ifeq ($(FAKEID),y)
	@echo '#define RT_SERIALNO "$(FORCE_SN)"' >> router/shared/version.h
ifneq ($(RCNO),)
	@echo '#define RT_RCNO "$(FORCE_RN)"' >> router/shared/version.h
else
	@echo '#define RT_RCNO NULL' >> router/shared/version.h
endif
	@echo '#define RT_EXTENDNO "$(FORCE_EN)$(BUILDREV)"' >> router/shared/version.h
else
	@echo '#define RT_SERIALNO "$(SERIALNO)"' >> router/shared/version.h
ifneq ($(RCNO),)
	@echo '#define RT_RCNO "$(RCNO)"' >> router/shared/version.h
else
	@echo '#define RT_RCNO NULL' >> router/shared/version.h
endif
	@echo '#define RT_EXTENDNO "$(EXTENDNO)$(BUILDREV)"' >> router/shared/version.h
endif
	@echo '#define RT_SWPJVERNO "$(SWPJVERNO)"' >> router/shared/version.h
	@echo '#define RT_BUILD_NAME "$(BUILD_NAME)"' >> router/shared/version.h
	@echo '#define RT_BUILD_INFO "$(BUILD_TIME) $(BUILD_USER)@$(BUILD_INFO)"' >> router/shared/version.h
	@echo '#endif' >> router/shared/version.h
	@echo '$(BUILD_NAME) $(SERIALNO)-$(EXTENDNO)$(BUILDREV) $(BUILD_TIME)' > router/shared/version
	@echo 'EXTENDNO=$(EXTENDNO)$(BUILDREV)' > router/extendno.conf


rt_ver_ntools:
	-@rm -f ntools/version
ifeq ($(FAKEID),y)
	-@echo 'KERNEL_IMAGE = $(BUILD_NAME)_$(KERNEL_VER).$(FS_VER)_$(FORCE_SN)_$(FORCE_EN).trx' >> ntools/version
else
	-@echo 'KERNEL_IMAGE = $(BUILD_NAME)_$(KERNEL_VER).$(FS_VER)_$(SERIALNO)_$(EXTENDNO).trx' >> ntools/version
endif

ifeq ($(HND_ROUTER),y)
all: cfe_sysdep wl_sysdep rt_ver mkenv prebuild_checks all_postcheck1
else
all: rt_ver rt_ver_ntools
endif
ifeq ($(RTCONFIG_REALTEK),y)
	@if [ -e $(SRCBASE)/build.log ]; then \
		mv $(SRCBASE)/build.log $(SRCBASE)/build.log.old; \
	fi;
	@echo "" > $(SRCBASE)/build.log
endif
	echo ""
	echo "Building $(BUILD_NAME)_$(KERNEL_VER).$(FS_VER)_$(SERIALNO).trx"
	echo ""
	echo ""

ifeq ($(HND_ROUTER),y)
	@rm -rf image
	@ln -sf targets/94908HND image

	$(MAKE) buildimage
else
	@-mkdir image

	$(MAKE) -C router all
ifneq ($(PLATFORM_ROUTER),)
	$(MAKE) -C $(PLATFORM_ROUTER) all
	$(MAKE) -C $(PLATFORM_ROUTER) install
endif
	$(MAKE) -C router install

	@$(MAKE) image
endif


rtk_clean_img:
ifeq ($(RTCONFIG_REALTEK),y)
	@rm -fr $(RSDKDIR)/target/romfs/*
	@mkdir -p $(RSDKDIR)/target/romfs
endif

rtk_clean_linux:
ifeq ($(RTCONFIG_REALTEK),y)
	@$(MAKE) -C $(RSDKDIR) clean
endif

configcheck:
	@echo "" > $(SRCBASE)/.diff_config;
	@if [ -e $(RSDKDIR)/users/boa/.kernel_config ]; then \
		diff $(RSDKDIR)/users/boa/.kernel_config $(LINUXDIR)/.config > $(SRCBASE)/.diff_config; \
		if [ -s $(SRCBASE)/.diff_config ]; then \
			cp $(LINUXDIR)/.config $(RSDKDIR)/users/boa/.kernel_config; \
			if [ -e $(RSDKDIR)/users/boa/tools/cvimg ]; then \
				rm $(RSDKDIR)/users/boa/tools/mgbin $(RSDKDIR)/users/boa/tools/mgbin.o $(RSDKDIR)/users/boa/tools/cvimg $(RSDKDIR)/users/boa/tools/cvimg.o; \
			fi;\
		fi; \
	else \
		cp $(LINUXDIR)/.config $(RSDKDIR)/users/boa/.kernel_config; \
	fi;

rtk_img_tool: configcheck
ifeq ($(RTL8197F),y)
	@$(MAKE) -C linux/rtl819x squashfs4.2/squashfs-tools-build
else
	@$(MAKE) -C linux/rtl819x squashfs4.0/squashfs-tools-build
endif
	@if [ -e $(RSDKDIR)/users/boa/tools/cvimg ]; then \
		echo "-------- boa exist  ----------------"; \
	else \
		$(MAKE) -C $(RSDKDIR) boa-build; \
	fi;

image:
	@if [ -z "$(BUILD_NAME)" ]; then \
		echo "No BUILD_NAME is assigned"; \
		exit 1; \
	fi

	@rm -f image/$(BUILD_NAME)_$(KERNEL_VER).$(FS_VER)_$(SERIALNO).trx
ifeq ($(RTCONFIG_REALTEK),y)
	@$(MAKE) rtk_clean_img
	@$(MAKE) rtk_img_tool
	@rsync -aPq $(PLATFORMDIR)/target/ $(RSDKDIR)/target/romfs
	@echo "-------- chmod busybox --------" >> $(SRCBASE)/build.log
	@chmod 755 $(RSDKDIR)/target/romfs/bin/busybox
else
	@$(MAKE) -C router image
endif

ifeq ($(CONFIG_RALINK),y)
# MTK/Ralink platform
	# generate kernel part
	@rm -rf $(PLATFORMDIR)/zImage.lzma ; \
	mipsel-linux-objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux $(PLATFORMDIR)/vmlinus ; \
	asustools/lzma_9k e $(PLATFORMDIR)/vmlinus $(PLATFORMDIR)/zImage.lzma -lc2 -lp2 -pb2 -mfbt2 ; \
	cp -f $(PLATFORMDIR)/zImage.lzma $(PLATFORMDIR)/zImage.img ; \
	# padded kernel image size
	@SIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'`; \
	if [ "`grep -c \"^CONFIG_ROOTFS_IN_FLASH_NO_PADDING\>\" $(LINUXDIR)/.config`" -eq 0 ] ; then \
		CONFIG_MTD_KERNEL_PART_SIZ=`grep "^CONFIG_MTD_KERNEL_PART_SIZ\>" $(LINUXDIR)/.config|sed -e "s,[^=]*=,," -e "s,^\(0x\)*,0x,"` ; \
		MTD_KRN_PART_SIZE=`printf "%d" $${CONFIG_MTD_KERNEL_PART_SIZ}` ; \
		PAD=`expr $${MTD_KRN_PART_SIZE} - 64 - $${SIZE}` ; \
		echo "S: $$SIZE $${MTD_KRN_PART_SIZE} $${PAD}" ; \
		if [ "$${PAD}" -le "0" ] ; then \
			echo "CONFIG_MTD_KERNEL_PART_SIZ $${CONFIG_MTD_KERNEL_PART_SIZ} is smaller than " \
				"`wc -c $(PLATFORMDIR)/zImage.img|awk '{printf "0x%x",$$1}'`. Increase it!" ; \
			ls -l $(PLATFORMDIR)/zImage.img ; \
			exit 1 ; \
		fi ; \
		dd if=/dev/zero count=1 bs=$${PAD} 2> /dev/null | tr \\000 \\377 >> $(PLATFORMDIR)/zImage.img ; \
	else  \
		PAD=`expr 64 - $${SIZE} % 64` ; \
		dd if=/dev/zero count=1 bs=$${PAD} 2> /dev/null | tr \\000 \\377 >> $(PLATFORMDIR)/zImage.img ; \
	fi ; \

	cat $(PLATFORMDIR)/target.image >> $(PLATFORMDIR)/zImage.img ; \
	#generate ASUS Image
	@ENTRY=`LANG=en_US readelf -h $(ROOTDIR)/$(LINUXDIR)/vmlinux | grep "Entry" | awk '{print $$4}'` ; \
	ISIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'` ; \
	KSIZE=`wc -c $(PLATFORMDIR)/zImage.lzma | awk '{print $$1}'` ; \
	RSIZE=`wc -c $(PLATFORMDIR)/target.image | awk '{print $$1}'` ; \
	PAD2=`expr $${ISIZE} - $${KSIZE} - $${RSIZE}` ; \
	RFSOFFSET=`expr 64 + $${KSIZE} + $${PAD2}` ; \
	echo "PAD2: $${PAD2}" ; \
	if [ "$(BUILD_NAME)" = "RT-N56UB1" ] || [ "$(BUILD_NAME)" = "RT-N56UB2" ] || [ "$(BUILD_NAME)" = "RT-AC1200GA1" ] || [ "$(BUILD_NAME)" = "RT-AC1200GU" ] || [ "$(BUILD_NAME)" = "RP-AC56" ] || [ "$(BUILD_NAME)" = "RP-AC87" ] || [ "$(BUILD_NAME)" = "RT-AC85U" ] || [ "$(BUILD_NAME)" = "RT-AC65U" ] || [ "$(BUILD_NAME)" = "RT-N800HP" ]; then \
		asustools/mkimage -A mips -O linux -T kernel -C lzma -a 80001000 -e $${ENTRY} -r $${RFSOFFSET} \
		-n $(BUILD_NAME) -V "$(KERNEL_VER)" "$(FS_VER)"  "$(SERIALNO)" "$(EXTENDNO)" "0" "0" "0" "0" "0" "0"  \
		-d $(PLATFORMDIR)/zImage.img image/$(IMGNAME).trx ; \
	else \
		asustools/mkimage -A mips -O linux -T kernel -C lzma -a 80000000 -e $${ENTRY} -r $${RFSOFFSET} \
		-n $(BUILD_NAME) -V "$(KERNEL_VER)" "$(FS_VER)" "$(SERIALNO)" "$(EXTENDNO)" "0" "0" "0" "0" "0" "0" \
		-d $(PLATFORMDIR)/zImage.img image/$(IMGNAME).trx ; \
	fi ; \
	if [ "`grep -c \"^CONFIG_RALINK_MT7620\>\" $(LINUXDIR)/.config`" -gt 0 ]; then \
		echo -n "PA/LNA: " ; \
		if [ "`grep -c \"^CONFIG_INTERNAL_PA_INTERNAL_LNA\>\" $(LINUXDIR)/.config`" -gt 0 ] ; then \
			echo "Internal PA + Internal LNA" ; \
			if [ "$(BUILD_NAME)" = "RT-N14U" ] || [ "$(BUILD_NAME)" = "RT-AC51U" ] || [ "$(BUILD_NAME)" = "RT-AC51U+" ] || [ "$(BUILD_NAME)" = "RT-N11P" ] || [ "$(BUILD_NAME)" = "RT-N300" ] || [ "$(BUILD_NAME)" = "RT-N54U" ] || [ "$(BUILD_NAME)" = "RT-AC54U" ] ; then \
				echo "Check PA/LNA: OK" ; \
			else \
				mv -f image/$(IMGNAME).trx image/$(IMGNAME)_int_PA_int_LNA.trx ; \
			fi ; \
		elif [ "`grep -c \"^CONFIG_INTERNAL_PA_EXTERNAL_LNA\>\" $(LINUXDIR)/.config`" -gt 0 ] ; then \
			echo "Internal PA + External LNA" ; \
			if [ "$(BUILD_NAME)" = "RT-AC1200HP" ]; then \
				echo "Check PA/LNA: OK" ; \
			else \
				mv -f image/$(IMGNAME).trx image/$(IMGNAME)_int_PA_ext_LNA.trx ; \
			fi ; \
		elif [ "`grep -c \"^CONFIG_EXTERNAL_PA_EXTERNAL_LNA\>\" $(LINUXDIR)/.config`" -gt 0 ] ; then \
			echo "External PA + External LNA" ; \
			if [ "$(BUILD_NAME)" = "RT-AC52U" ] || [ "$(BUILD_NAME)" = "RT-AC53" ]; then \
				echo "Check PA/LNA: OK" ; \
			else \
				mv -f image/$(IMGNAME).trx image/$(IMGNAME)_ext_PA_ext_LNA.trx ; \
			fi ; \
		else \
			echo "UNKNOWN PA/LNA" ; \
		fi ; \
	fi
else ifeq ($(QCA),y)
# Qualcomm Atheros platform
ifeq ($(and $(LOADADDR),$(ENTRYADDR)),)
	$(error Unknown load/entry address)
endif
	# generate kernel part
ifeq ($(IPQ40XX),y)
	@rm -rf $(PLATFORMDIR)/zImage.lzma ; \
		$(CROSS_COMPILE)objcopy -O binary $(LINUXDIR)/vmlinux $(PLATFORMDIR)/vmlinus ; \
		asustools/lzma -9 -f -c $(PLATFORMDIR)/vmlinus > $(PLATFORMDIR)/zImage.lzma ;
	cp -f $(PLATFORMDIR)/zImage.lzma $(PLATFORMDIR)/zImage.img ; \
	# padded kernel image size
	@SIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'`; \
		PAD=`expr 64 - $${SIZE} % 64` ; \
		dd if=/dev/zero count=1 bs=$${PAD} 2> /dev/null | tr \\000 \\377 >> $(PLATFORMDIR)/zImage.img
	asustools/mkits.sh -D qcom-ipq40xx-ap.dkxx -o image/fit-qcom-ipq40xx-ap.dkxx.its -k $(PLATFORMDIR)/zImage.img -r $(PLATFORMDIR)/target.image -d $(LINUXDIR)/arch/arm/boot/dts/qcom-ipq40xx-$(lowercase_B).dtb -C lzma -a 0x$(LOADADDR) -e 0x$(ENTRYADDR) -A $(ARCH) -v $(KERNEL_VER)
	asustools/mkimage -f image/fit-qcom-ipq40xx-ap.dkxx.its image/$(IMGNAME).img
	#generate ASUS Image
	@ISIZE=`wc -c image/$(IMGNAME).img | awk '{print $$1}'` ; \
		KSIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'` ; \
		RSIZE=`wc -c $(PLATFORMDIR)/target.image | awk '{print $$1}'` ; \
		PAD2=`expr $${ISIZE} - $${KSIZE} - $${RSIZE}` ; \
		RFSOFFSET=`expr 64 + $${KSIZE} + $${PAD2}` ; \
		echo "PAD2: $${PAD2}" ; \
	asustools/mkimage -A $(ARCH) -O linux -T kernel -C lzma -a $(LOADADDR) -e $(ENTRYADDR) -r $${RFSOFFSET} \
			-n $(BUILD_NAME) -V "$(KERNEL_VER)" "$(FS_VER)" "$(SERIALNO)" "$(EXTENDNO)" "0" "0" "0" "0" "0" "0" \
			-d image/$(IMGNAME).img image/$(IMGNAME).trx
else
	@rm -rf $(PLATFORMDIR)/zImage.lzma ; \
		$(CROSS_COMPILE)objcopy -O binary $(LINUXDIR)/vmlinux $(PLATFORMDIR)/vmlinus ; \
		asustools/lzma -9 -f -c $(PLATFORMDIR)/vmlinus > $(PLATFORMDIR)/zImage.lzma ; \
		cp -f $(PLATFORMDIR)/zImage.lzma $(PLATFORMDIR)/zImage.img ; \
	# padded kernel image size
	@SIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'`; \
		PAD=`expr 64 - $${SIZE} % 64` ; \
		dd if=/dev/zero count=1 bs=$${PAD} 2> /dev/null | tr \\000 \\377 >> $(PLATFORMDIR)/zImage.img
	@cat $(PLATFORMDIR)/target.image >> $(PLATFORMDIR)/zImage.img ; \
	#generate ASUS Image
	@ISIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'` ; \
		KSIZE=`wc -c $(PLATFORMDIR)/zImage.lzma | awk '{print $$1}'` ; \
		RSIZE=`wc -c $(PLATFORMDIR)/target.image | awk '{print $$1}'` ; \
		PAD2=`expr $${ISIZE} - $${KSIZE} - $${RSIZE}` ; \
		RFSOFFSET=`expr 64 + $${KSIZE} + $${PAD2}` ; \
		echo "PAD2: $${PAD2}" ; \
		asustools/mkimage -A $(ARCH) -O linux -T kernel -C lzma -a $(LOADADDR) -e $(ENTRYADDR) -r $${RFSOFFSET} \
			-n $(BUILD_NAME) -V "$(KERNEL_VER)" "$(FS_VER)" "$(SERIALNO)" "$(EXTENDNO)" "0" "0" "0" "0" "0" "0" \
			-d $(PLATFORMDIR)/zImage.img image/$(IMGNAME).trx
endif
else ifeq ($(RTCONFIG_REALTEK),y)
# Realtek platform
	@$(MAKE) -C linux/rtl819x image
	@rm -f $(SRCBASE)/image-realtek
	@ln -sf $(RSDKDIR)/image $(SRCBASE)/image-realtek
else ifeq ($(ALPINE),y)
	$(MAKE) -C ctools clean
	$(MAKE) -C ctools TRX=NEW
	$(MAKE) -C proprietary gen_uimage-clean
	$(MAKE) -C proprietary gen_uimage
	# Create generic TRX image
	# generate kernel part
	@rm -rf $(PLATFORMDIR)/zImage.lzma ; \
		$(CROSS_COMPILE)objcopy -O binary $(LINUXDIR)/vmlinux $(PLATFORMDIR)/vmlinus ; \
		asustools/lzma -9 -f -c $(PLATFORMDIR)/vmlinus > $(PLATFORMDIR)/zImage.lzma ; \
		cp -f $(PLATFORMDIR)/zImage.lzma $(PLATFORMDIR)/zImage.img ; \
	# padded kernel image size
	@SIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'`; \
		PAD=`expr 64 - $${SIZE} % 64` ; \
		dd if=/dev/zero count=1 bs=$${PAD} 2> /dev/null | tr \\000 \\377 >> $(PLATFORMDIR)/zImage.img
	@cat $(PLATFORMDIR)/target.image >> $(PLATFORMDIR)/zImage.img ; \
	#generate ASUS Image
	@ISIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'` ; \
		KSIZE=`wc -c $(PLATFORMDIR)/zImage.lzma | awk '{print $$1}'` ; \
		RSIZE=`wc -c $(PLATFORMDIR)/target.image | awk '{print $$1}'` ; \
		PAD2=`expr $${ISIZE} - $${KSIZE} - $${RSIZE}` ; \
		RFSOFFSET=`expr 64 + $${KSIZE} + $${PAD2}` ; \
		echo "PAD2: $${PAD2}" ; \
		asustools/mkimage -A $(ARCH) -O linux -T kernel -C lzma -a $(LOADADDR) -e $(ENTRYADDR) -r $${RFSOFFSET} \
			-n $(BUILD_NAME) -V "$(KERNEL_VER)" "$(FS_VER)" "0" "0" "0" "0" "0" "0" "0" "0" \
			-d $(PLATFORMDIR)/zImage.img image/$(IMGNAME).trx
	@proprietary/gen_uimage
	dd if=/dev/zero count=1 bs=128 | tr "\000" "\145" > ./eof.txt
	rm -f image/tmp-linux.trx; ctools/trx -o image/tmp-linux.trx $(PLATFORMDIR)/uImage.final $(PLATFORMDIR)/target.image ./eof.txt
	ctools/trx_asus -i image/tmp-linux.trx -r $(BUILD_NAME),$(KERNEL_VER).$(FS_VER),$(SERIALNO),$(EXTENDNO),image/$(IMGNAME).trx
	rm -f ./eof.txt
else ifeq ($(LANTIQ),y)
	$(MAKE) -C ctools clean
	$(MAKE) -C ctools TRX=NEW
	# generate kernel part
	@rm -rf $(PLATFORMDIR)/zImage.lzma ; \
		$(CROSS_COMPILE)objcopy -O binary $(LINUXDIR)/vmlinux $(PLATFORMDIR)/vmlinus ; \
		asustools/lzma -9 -f -c $(PLATFORMDIR)/vmlinus > $(PLATFORMDIR)/zImage.lzma ; \
		cp -f $(PLATFORMDIR)/zImage.lzma $(PLATFORMDIR)/zImage.img ; \
	# padded kernel image size
	@SIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'`; \
		PAD=`expr 64 - $${SIZE} % 64` ; \
		dd if=/dev/zero count=1 bs=$${PAD} 2> /dev/null | tr \\000 \\377 >> $(PLATFORMDIR)/zImage.img
	@cat $(PLATFORMDIR)/target.image >> $(PLATFORMDIR)/zImage.img ; \
	#generate ASUS Image
	@ISIZE=`wc -c $(PLATFORMDIR)/zImage.img | awk '{print $$1}'` ; \
		KSIZE=`wc -c $(PLATFORMDIR)/zImage.lzma | awk '{print $$1}'` ; \
		RSIZE=`wc -c $(PLATFORMDIR)/target.image | awk '{print $$1}'` ; \
		PAD2=`expr $${ISIZE} - $${KSIZE} - $${RSIZE}` ; \
		RFSOFFSET=`expr 64 + $${KSIZE} + $${PAD2}` ; \
		echo "PAD2: $${PAD2}" ; \
		asustools/mkimage -A $(ARCH) -O linux -T kernel -C lzma -a $(LOADADDR) -e $(ENTRYADDR) -r $${RFSOFFSET} \
			-n $(BUILD_NAME) -V "$(KERNEL_VER)" "$(FS_VER)" "0" "0" "0" "0" "0" "0" "0" "0" \
			-d $(PLATFORMDIR)/zImage.img image/$(IMGNAME).trx
	$(call build_bluecave_image)
else
# Broadcom ARM/MIPS platform
	$(MAKE) -C ctools clean
	$(MAKE) -C ctools $(if $(CONFIG_BCMWL5),TRX=NEW,)
	# Create generic TRX image
ifeq ($(ARM),y)
	ctools/objcopy -O binary -R .note -R .note.gnu.build-id -R .comment -S $(LINUXDIR)/vmlinux $(PLATFORMDIR)/piggy
else
	ctools/objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S $(LINUXDIR)/vmlinux $(PLATFORMDIR)/piggy
endif
ifneq (,$(filter y,$(BCMWL6) $(BCMWL6A) $(BOOTLZMA) $(ARM)))
	ctools/lzma_4k e $(PLATFORMDIR)/piggy $(PLATFORMDIR)/vmlinuz-lzma
	ctools/trx -o image/linux-lzma.trx $(PLATFORMDIR)/vmlinuz-lzma $(PLATFORMDIR)/target.image
else
	ctools/lzma_9k e $(PLATFORMDIR)/piggy $(PLATFORMDIR)/vmlinuz-lzma -eos -lc2 -lp2 -pb2 -mfbt2
	ctools/trx -o image/linux-lzma.trx lzma-loader/loader.gz $(PLATFORMDIR)/vmlinuz-lzma $(PLATFORMDIR)/target.image
endif
ifneq ($(CONFIG_BCMWL5),)
ifeq ($(FAKEHDR),y)
	ctools/trx_asus -i image/linux-lzma.trx -r $(BUILD_NAME),$(KERNEL_VER).$(FS_VER),$(FORCE_SN),$(FORCE_EN),image/$(IMGNAME).trx
else
	ctools/trx_asus -i image/linux-lzma.trx -r $(BUILD_NAME),$(KERNEL_VER).$(FS_VER),$(SERIALNO),$(EXTENDNO),image/$(IMGNAME).trx
endif
else
	ctools/trx_asus -i image/linux-lzma.trx -r $(BUILD_NAME),$(KERNEL_VER).$(FS_VER),image/$(IMGNAME).trx
endif
	@rm -f image/linux-lzma.trx
endif

ifeq ($(RTCONFIG_REALTEK),y)
	ENTRY=`LANG=en_US readelf -h linux/rtl819x/linux-3.10/vmlinux | grep "Entry" | awk '{print $$4}'` ; \
	asustools/mkimage -A mips -O linux -T kernel -C lzma -a 80c00000 -e $${ENTRY} -n $(BUILD_NAME) -V "$(KERNEL_VER)" "$(FS_VER)" "$(SERIALNO)" "$(EXTENDNO)" "0" "0" "0" "0" "0" "0" -d $(RSDKDIR)/image/fw.bin image/$(IMGNAME).trx
	#@asustools/mkimage -A mips -O linux -d $(RSDKDIR)/image/fw.bin image/$(IMGNAME).trx
endif
	#md5sum image/$(IMGNAME).trx > image/$(IMGNAME).md5

ifeq ($(DSL),y)
	$(call dsl_genbintrx_epilog)
ifeq ($(DSL_TCLINUX),y)
	cat image/$(IMGNAME).trx tc_fw/tclinux.bin > image/$(DSLIMGNAME).trx
endif

	md5sum image/$(DSLIMGNAME).trx > image/$(DSLIMGNAME).md5
ifeq ($(HTTPS),y)
ifneq ("$(wildcard $(SRC_ROOT)/../../../buildtools/private.pem)","")
	openssl sha1 -sign $(SRC_ROOT)/../../../buildtools/private.pem -out image/$(DSLIMGNAME)_rsa.zip image/$(DSLIMGNAME).trx
endif
endif
endif

#general rsasign file
ifeq ($(HTTPS),y)
ifneq ("$(wildcard $(SRC_ROOT)/../../../buildtools/private.pem)","")
	openssl sha1 -sign $(SRC_ROOT)/../../../buildtools/private.pem -out image/$(IMGNAME)_rsa.zip image/$(IMGNAME).trx
endif
endif
#	ln -sf $(IMGNAME).trx image/$(BUILD_NAME).trx
#ifeq ($(ARM),y)
#	ln -sf $(LINUXDIR)/vmlinux image/vmlinux
#	ln -sf $(LINUXDIR)/vmlinux.o image/vmlinux.o
#endif
	@echo ""

export_config:
	@-mkdir image/log
	sh $(SRCBASE)/../../buildtools/parseconfig.sh $(SRCBASE)/../.. $(KERNEL_VER).$(FS_VER)_$(SERIALNO)_$(EXTENDNO) $(SRCBASE)/image $(BUILD_NAME)

kernel_patch:
ifeq ($(CONFIG_LANTIQ),y)
	cd proprietary; rm -rf ltq_eip97_1.2.25; tar zxf ltq_eip97_1.2.25.tar.gz
	cd proprietary; rm -rf ${SRCBASE}/router/rom_lantiq/; tar zxf rom_lantiq_05.04.00.61.7_AE.tgz -C ${SRCBASE}/router/ ; mv ${SRCBASE}/router/rom_lantiq_05.04.00.61.7_AE ${SRCBASE}/router/rom_lantiq/
	cp patches/iptables-1.4.21/extensions/* ${SRCBASE}/router/iptables-1.4.21/extensions/
	# rm -f ${SRCBASE}/router/rom_lantiq/opt/lantiq/wave/images/cal_wlan0.bin
	# rm -f ${SRCBASE}/router/rom_lantiq/opt/lantiq/wave/images/cal_wlan1.bin
	# rm -f proprietary/rom_lantiq/opt/lantiq/wave/images/cal_wlan0.bin
	# rm -f proprietary/rom_lantiq/opt/lantiq/wave/images/cal_wlan1.bin
	cd proprietary; cp -rf rom_lantiq_05.04.00.61.7_AE_patch/* ${SRCBASE}/router/rom_lantiq/
	# cp -f proprietary/linux-3.10.104/firmware/lantiq/phy11g_ip_BE.bin linux/linux-3.10.104/firmware/lantiq/phy11g_ip_BE.bin
endif

prepare_toolchain:
	if [ ! -d "${SRCBASE}/tools/${TOOLCHAIN_NAME}" ] ; then \
		cd ${SRCBASE}/tools; tar zxf ${TOOLCHAIN_NAME}.tgz ; \
	fi
	if [ ! -d "${SRCBASE}/tools/${TOOLCHAIN_HOST_NAME}" ] ; then \
		if [ "`uname -m`" = "x86_64" ] ; then \
			cd ${SRCBASE}/tools; tar zxf ${TOOLCHAIN_HOST_NAME}_64bit.tgz ; \
		else \
			cd ${SRCBASE}/tools; tar zxf ${TOOLCHAIN_HOST_NAME}.tgz ; \
		fi \
	fi
ifeq ($(CONFIG_LANTIQ),y)
	if [ ! -f "${SRCBASE}/tools/${TOOLCHAIN_NAME}/.patched" ] ; then \
		cd ${SRCBASE}/tools/${TOOLCHAIN_NAME}; patch -p1 < ${SRCBASE}/patches/toolchain/0001-rp-pppoe_if_pppox.patch ; \
	fi
endif
	if [ ! -f "${SRCBASE}/tools/${TOOLCHAIN_NAME}/.patched" ] ; then \
		cd ${SRCBASE}/tools/${TOOLCHAIN_NAME}/lib; cp ${SRCBASE}/patches/toolchain/lib/libnsl-0.9.33.2.so .; cp ${SRCBASE}/patches/toolchain/lib/libresolv-0.9.33.2.so . ; ln -s libnsl-0.9.33.2.so libnsl.so.0; ln -s libnsl.so.0 libnsl.so ; ln -s libresolv-0.9.33.2.so libresolv.so.0 ; \
		touch ${SRCBASE}/tools/${TOOLCHAIN_NAME}/.patched ; \
	fi
ifeq ($(CONFIG_LANTIQ),y)
	rm -rf tools/${TOOLCHAIN_HOST_NAME}/include/linux/
endif

rt-ac98u_kernel:
	@echo ${SRCBASE}
	@echo ${TOOLCHAIN}
	if [ ! -d "${SRCBASE}/tools/${TOOLCHAIN_NAME}" ] ; then \
		cd ${SRCBASE}/tools; tar zxf ${TOOLCHAIN_NAME}.tgz ; \
	fi
	if [ ! -d "${SRCBASE}/tools/${TOOLCHAIN_HOST_NAME}" ] ; then \
		cd ${SRCBASE}/tools; tar zxf ${TOOLCHAIN_HOST_NAME}.tgz ; \
	fi
	# uImage (Host Linux)
	@echo ${STAGING_DIR}/${TOOLCHAIN_NAME}/bin
	@cp ${LINUXDIR}/config_base ${LINUXDIR}/.config
	make -j 9 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${STAGING_DIR}/host/include -Wall -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_SHELL="/bin/bash" V='' uImage
	make -j 9 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${STAGING_DIR}/host/include -Wall -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_SHELL="/bin/bash" V='' modules

bluecave_kernel:
	@echo ${SRCBASE}
	@echo ${TOOLCHAIN}
	if [ ! -d "${SRCBASE}/tools/${TOOLCHAIN_NAME}" ] ; then \
		cd ${SRCBASE}/tools; tar zxf ${TOOLCHAIN_NAME}.tgz ; \
	fi
	if [ ! -d "${SRCBASE}/tools/${TOOLCHAIN_HOST_NAME}" ] ; then \
		cd ${SRCBASE}/tools; tar zxf ${TOOLCHAIN_HOST_NAME}.tgz ; \
	fi
	# uImage (Host Linux)
	@echo ${STAGING_DIR}/${TOOLCHAIN_NAME}/bin
	make -j3 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${SRCBASE}/tools/host/include -I${SRCBASE}/tools/host/usr/include -Wall -Wmissing-prototypes -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_DEBUG_SECTION_MISMATCH=y CONFIG_SHELL="/bin/bash" V='' INSTALL_HDR_PATH=${LINUXDIR}/user_headers headers_install
	make -j3 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${SRCBASE}/tools/host/include -I${SRCBASE}/tools/host/usr/include -Wall -Wmissing-prototypes -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_DEBUG_SECTION_MISMATCH=y CONFIG_SHELL="/bin/bash" V='' INSTALL_HDR_PATH=${LINUXDIR} modules
	make -j3 -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${SRCBASE}/tools/host/include -I${SRCBASE}/tools/host/usr/include -Wall -Wmissing-prototypes -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_DEBUG_SECTION_MISMATCH=y CONFIG_SHELL="/bin/bash" V='' INSTALL_HDR_PATH=${LINUXDIR} all modules
	# make -C ${LINUXDIR} HOSTCFLAGS="-O2 -I${STAGING_DIR}/host/include -Wall -Wmissing-prototypes -Wstrict-prototypes" KBUILD_HAVE_NLS=no CONFIG_SHELL="/bin/bash" V='' modules


ifeq ($(IPQ40XX),y)
bin_file:
	if [ ! -e image/$(IMGNAME).img ]; then \
		[ -e image/$(IMGNAME).trx ] && tail -c +65 image/$(IMGNAME).trx > image/$(IMGNAME).img ; \
	fi
	[ ! -e .bin/$(BUILD_NAME)/Makefile ] || $(MAKE) -C .bin/$(BUILD_NAME) O=$(SRCBASE)/image FW_FN=$(IMGNAME).img
	if [ "$(BUILD_NAME)" = "RT-AC58U" ]; then \
		[ ! -e .bin/$(BUILD_NAME)/Makefile ] || $(MAKE) -C .bin/$(BUILD_NAME) O=$(SRCBASE)/image FW_FN=$(IMGNAME).img CE=y; \
		[ ! -e .bin/$(BUILD_NAME)/Makefile ] || $(MAKE) -C .bin/$(BUILD_NAME) O=$(SRCBASE)/image FW_FN=$(IMGNAME).img RTAC1300UHP=y; \
		[ ! -e .bin/$(BUILD_NAME)/Makefile ] || $(MAKE) -C .bin/$(BUILD_NAME) O=$(SRCBASE)/image FW_FN=$(IMGNAME).img RTAC1300UHP=y CE=y; \
	fi
else
bin_file:
	[ ! -e .bin/$(BUILD_NAME)/Makefile ] || $(MAKE) -C .bin/$(BUILD_NAME) O=$(SRCBASE)/image FW_FN=$(IMGNAME).trx
endif

ifneq ($(HND_ROUTER),y)
clean: rtk_clean_img rtk_clean_linux
ifneq ($(PLATFORM_ROUTER),)
	$(MAKE) -C $(PLATFORM_ROUTER) $@
endif
	@touch router/.config
	@rm -f router/config_[a-z]
	@rm -f router/$(BUSYBOX)/config_[a-z]
	@-$(MAKE) -C router $@
	@-$(MAKE) -C $(LINUXDIR) $@
	@-$(MAKE) cleantools
	@-rm -rf $(PLATFORMDIR)
endif

cleanimage: rtk_clean_img
	@rm -f fpkg.log
	@rm -fr image/*
	@rm -f router/.config
	@touch router/.config
ifneq ($(HND_ROUTER),y)
	@-mkdir image
endif

cleantools:
	@[ ! -d $(LINUXDIR)/scripts/squashfs ] || \
		$(MAKE) -C $(LINUXDIR)/scripts/squashfs clean
	@$(MAKE) -C btools clean
ifeq ($(CONFIG_RALINK),y)
else ifeq ($(CONFIG_QCA),y)
else ifeq ($(REALTEK),y)
else
	@$(MAKE) -C ctools clean
endif

cleankernel:
	@cd $(LINUXDIR) && \
	mv .config save-config && \
	$(MAKE) distclean || true; \
	cp -p save-config .config || true

kernel:	
	$(MAKE) -C router kernel
	@[ ! -e $(KERNEL_BINARY) ] || ls -l $(KERNEL_BINARY)

distclean: clean cleanimage cleankernel cleantools cleanlibc
ifneq ($(INSIDE_MAK),1)
	@$(MAKE) -C router $@ INSIDE_MAK=1
endif
	mv router/$(BUSYBOX)/.config busybox-saved-config || true
	@$(MAKE) -C router/$(BUSYBOX) distclean
	@rm -f router/$(BUSYBOX)/config_current
	@cp -p busybox-saved-config router/$(BUSYBOX)/.config || true
	@cp -p router/$(BUSYBOX)/.config  router/$(BUSYBOX)/config_current || true
	@rm -f router/config_current
	@rm -f router/.config.cmd router/.config.old router/.config
	@rm -f router/libfoo_xref.txt
	@-rm -f .config

prepk:
	@cd $(LINUXDIR) ; \
		rm -f config_current ; \
		ln -s config_base config_current ; \
		cp -f config_current .config
ifeq ($(CONFIG_LINUX26),y)
	$(MAKE) -C $(LINUXDIR) oldconfig prepare
else
	$(MAKE) -C $(LINUXDIR) oldconfig dep
endif

what:
	@echo ""
	@echo "$(current_BUILD_DESC)-$(current_BUILD_NAME)-$(TOMATO_PROFILE_NAME) Profile"
	@echo ""

# The methodology for making the different builds is to
# copy the "base" config file to the "target" config file in
# the appropriate directory, and then edit it by removing and
# inserting the desired configuration lines.
# You can't just delete the "whatever=y" line, you must have
# a "...is not set" line, or the make oldconfig will stop and ask
# what to do.

# Options for "make bin" :
# BUILD_DESC (Std|Lite|Ext|...)
# MIPS32 (r2|r1)
# KERN_SIZE_OPT
# USB ("USB"|"")
# JFFSv1 | NO_JFFS
# NO_CIFS, NO_SSH, NO_ZEBRA, NO_SAMBA, NO_FTP, NO_LIBOPT
# SAMBA3, OPENVPN, IPSEC, IPV6SUPP, EBTABLES, NTFS, MEDIASRV, BBEXTRAS, USBEXTRAS, BCM57, SLIM, XHCI, PSISTLOG
# STRACE, GDB

IPSEC_ID_POOL =		\
	"QUICKSEC"	\
	"STRONGSWAN"

define RouterOptions
	@( \
	if [ "$(CONFIG_LINUX26)" = "y" ] ; then \
		if [ "$(SAMBA3)" = "3.6.x" ]; then \
			sed -i "/RTCONFIG_SAMBA36X/d" $(1); \
			echo "RTCONFIG_SAMBA36X=y" >>$(1); \
			sed -i "/RTCONFIG_SAMBA3\>/d" $(1); \
			echo "# RTCONFIG_SAMBA3 is not set" >>$(1); \
		else \
			sed -i "/RTCONFIG_SAMBA36X/d" $(1); \
			echo "# RTCONFIG_SAMBA36X is not set" >>$(1); \
			sed -i "/RTCONFIG_SAMBA3\>/d" $(1); \
			echo "RTCONFIG_SAMBA3=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(REALTEK)" = "y" ]; then \
		sed -i "/RTCONFIG_REALTEK/d" $(1); \
		echo "RTCONFIG_REALTEK=y" >>$(1); \
	fi; \
	if [ "$(RTL819X)" = "y" ]; then \
		sed -i "/RTCONFIG_RTL819X/d" $(1); \
		echo "RTCONFIG_RTL819X=y" >>$(1); \
	else \
		echo "# RTCONFIG_RTL819X is not set" >>$(1); \
	fi; \
	if [ "$(RTL8197F)" = "y" ]; then \
		sed -i "/RTCONFIG_RTL8197F/d" $(1); \
		echo "RTCONFIG_RTL8197F=y" >>$(1); \
	else\
		echo "# RTCONFIG_RTL8197F is not set" >>$(1); \
	fi; \
	if [ "$(RTK_NAND)" = "y" ]; then \
		sed -i "/RTCONFIG_RTK_NAND/d" $(1); \
		echo "RTCONFIG_RTK_NAND=y" >>$(1); \
	else \
		echo "# RTCONFIG_RTK_NAND is not set" >>$(1); \
	fi; \
	if [ "$(CONFIG_BCMWL5)" = "y" ]; then \
		sed -i "/CONFIG_LIBBCM/d" $(1); \
		echo "CONFIG_LIBBCM=y" >>$(1); \
		sed -i "/CONFIG_LIBUPNP/d" $(1); \
		echo "CONFIG_LIBUPNP=y" >>$(1); \
	fi; \
	sed -i "/RTCONFIG_EMF/d" $(1); \
	if [ "$(CONFIG_LINUX26)" = "y" ]; then \
		if [ "$(SLIM)" = "y" ]; then \
			echo "# RTCONFIG_EMF is not set" >>$(1); \
		else \
			echo "RTCONFIG_EMF=y" >>$(1); \
		fi; \
	else \
		echo "# RTCONFIG_EMF is not set" >>$(1); \
	fi; \
	sed -i "/RTCONFIG_JFFSV1/d" $(1); \
	if [ "$(CONFIG_LINUX26)" = "y" ]; then \
		if [ "$(JFFSv1)" = "y" ]; then \
			echo "RTCONFIG_JFFSV1=y" >>$(1); \
		else \
			echo "# RTCONFIG_JFFSV1 is not set" >>$(1); \
		fi; \
	else \
		echo "RTCONFIG_JFFSV1=y" >>$(1); \
	fi; \
	if [ "$(YAFFS)" = "y" ]; then \
		sed -i "/RTCONFIG_YAFFS/d" $(1); \
		echo "RTCONFIG_YAFFS=y" >>$(1); \
		sed -i "/RTCONFIG_JFFS2/d" $(1); \
		echo "# RTCONFIG_JFFS2 is not set" >>$(1); \
		sed -i "/RTCONFIG_JFFSV1/d" $(1); \
		echo "# RTCONFIG_JFFSV1 is not set" >>$(1); \
	fi; \
	if [ "$(FANCTRL)" = "y" ]; then \
		sed -i "/RTCONFIG_FANCTRL/d" $(1); \
		echo "RTCONFIG_FANCTRL=y" >>$(1); \
	fi; \
	if [ "$(BCM5301X)" = "y" ]; then \
		sed -i "/RTCONFIG_5301X/d" $(1); \
		echo "RTCONFIG_5301X=y" >>$(1); \
	fi; \
	if [ "$(BCMWL6)" = "y" ]; then \
		sed -i "/RTCONFIG_BCMWL6/d" $(1); \
		echo "RTCONFIG_BCMWL6=y" >>$(1); \
		sed -i "/RTCONFIG_BCMDCS/d" $(1); \
		echo "RTCONFIG_BCMDCS=y" >>$(1); \
	fi; \
	if [ "$(BCMWL6A)" = "y" ]; then \
		sed -i "/RTCONFIG_BCMWL6A/d" $(1); \
		echo "RTCONFIG_BCMWL6A=y" >>$(1); \
	fi; \
	if [ "$(ARM)" = "y" ]; then \
		sed -i "/RTCONFIG_BCMARM/d" $(1); \
		echo "RTCONFIG_BCMARM=y" >>$(1); \
	fi; \
	if [ "$(ALPINE)" = "y" ]; then \
		sed -i "/RTCONFIG_ALPINE/d" $(1); \
		echo "RTCONFIG_ALPINE=y" >>$(1); \
	fi; \
	if [ "$(LANTIQ)" = "y" ]; then \
		sed -i "/RTCONFIG_LANTIQ/d" $(1); \
		echo "RTCONFIG_LANTIQ=y" >>$(1); \
	fi; \
	if [ "$(QSR10G)" = "y" ]; then \
		sed -i "/RTCONFIG_QSR10G/d" $(1); \
		echo "RTCONFIG_QSR10G=y" >>$(1); \
	fi; \
	if [ "$(NVRAM_FILE)" = "y" ]; then \
		sed -i "/RTCONFIG_NVRAM_FILE/d" $(1); \
		echo "RTCONFIG_NVRAM_FILE=y" >>$(1); \
	fi; \
	if [ "$(BCMSMP)" = "y" ]; then \
		sed -i "/RTCONFIG_BCMSMP/d" $(1); \
		echo "RTCONFIG_BCMSMP=y" >>$(1); \
	fi; \
	if [ "$(BCMFA)" = "y" ]; then \
		sed -i "/RTCONFIG_BCMFA/d" $(1); \
		echo "RTCONFIG_BCMFA=y" >>$(1); \
	fi; \
	if [ "$(RGMII_BCM_FA)" = "y" ]; then \
		sed -i "/RTCONFIG_RGMII_BCM_FA/d" $(1); \
		echo "RTCONFIG_RGMII_BCM_FA=y" >>$(1); \
	fi; \
	if [ "$(COMA)" = "y" ]; then \
		sed -i "/RTCONFIG_COMA/d" $(1); \
		echo "RTCONFIG_COMA=y" >>$(1); \
	fi; \
	if [ "$(WIRELESSWAN)" = "y" ]; then \
		sed -i "/RTCONFIG_WIRELESSWAN/d" $(1); \
		echo "RTCONFIG_WIRELESSWAN=y" >>$(1); \
	fi; \
	if [ "$(PARENTAL2)" = "y" -o "$(PARENTAL)" = "y" ]; then \
		sed -i "/RTCONFIG_PARENTALCTRL/d" $(1); \
		echo "RTCONFIG_PARENTALCTRL=y" >>$(1); \
	fi; \
	if [ "$(YANDEXDNS)" = "y" ]; then \
		sed -i "/RTCONFIG_YANDEXDNS/d" $(1); \
		echo "RTCONFIG_YANDEXDNS=y" >>$(1); \
	fi; \
	if [ "$(PPTPD)" = "y" ]; then \
		sed -i "/RTCONFIG_PPTPD/d" $(1); \
		echo "RTCONFIG_PPTPD=y" >>$(1); \
	fi; \
	if [ "$(REPEATER)" = "y" ]; then \
		sed -i "/RTCONFIG_WIRELESSREPEATER/d" $(1); \
		echo "RTCONFIG_WIRELESSREPEATER=y" >>$(1); \
		if [ "$(DISABLE_REPEATER_UI)" = "y" ] ; then \
			sed -i "/RTCONFIG_DISABLE_REPEATER_UI/d" $(1); \
			echo "RTCONFIG_DISABLE_REPEATER_UI=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(PURE_REPEATER)" = "y" ]; then \
		sed -i "/RTCONFIG_REPEATER/d" $(1); \
		echo "RTCONFIG_REPEATER=y" >>$(1); \
		if [ "$(DISABLE_REPEATER_UI)" = "y" ] ; then \
			sed -i "/RTCONFIG_DISABLE_REPEATER_UI/d" $(1); \
			echo "RTCONFIG_DISABLE_REPEATER_UI=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(PROXYSTA)" = "y" ]; then \
		sed -i "/RTCONFIG_PROXYSTA/d" $(1); \
		echo "RTCONFIG_PROXYSTA=y" >>$(1); \
	fi; \
	if [ "$(DISABLE_PROXYSTA_UI)" = "y" ] ; then \
		sed -i "/RTCONFIG_DISABLE_PROXYSTA_UI/d" $(1); \
		echo "RTCONFIG_DISABLE_PROXYSTA_UI=y" >>$(1); \
	fi; \
	if [ "$(PSR_GUEST)" = "y" ]; then \
		sed -i "/RTCONFIG_PSR_GUEST/d" $(1); \
		echo "RTCONFIG_PSR_GUEST=y" >>$(1); \
	fi; \
	if [ "$(CONCURRENTREPEATER)" = "y" ]; then \
		sed -i "/RTCONFIG_CONCURRENTREPEATER/d" $(1); \
		echo "RTCONFIG_CONCURRENTREPEATER=y" >>$(1); \
	fi; \
	if [ "$(IXIAEP)" = "y" ]; then \
		sed -i "/RTCONFIG_IXIAEP/d" $(1); \
		echo "RTCONFIG_IXIAEP=y" >>$(1); \
	fi; \
	if [ "$(IPERF)" = "y" ]; then \
		sed -i "/RTCONFIG_IPERF/d" $(1); \
		echo "RTCONFIG_IPERF=y" >>$(1); \
	fi; \
	if [ "$(RGBLED)" = "y" ]; then \
		sed -i "/RTCONFIG_RGBLED/d" $(1); \
		echo "RTCONFIG_RGBLED=y" >>$(1); \
	fi; \
	if [ "$(I2CTOOLS)" = "y" ]; then \
		sed -i "/RTCONFIG_I2CTOOLS/d" $(1); \
		echo "RTCONFIG_I2CTOOLS=y" >>$(1); \
	fi; \
	if [ "$(TCPDUMP)" = "y" ]; then \
		sed -i "/RTCONFIG_TCPDUMP/d" $(1); \
		echo "RTCONFIG_TCPDUMP=y" >>$(1); \
	fi; \
	if [ "$(TRACEROUTE)" = "y" ]; then \
		sed -i "/RTCONFIG_TRACEROUTE/d" $(1); \
		echo "RTCONFIG_TRACEROUTE=y" >>$(1); \
	fi; \
	if [ "$(NETOOL)" = "y" ]; then \
		sed -i "/RTCONFIG_NETOOL/d" $(1); \
		echo "RTCONFIG_NETOOL=y" >>$(1); \
	fi; \
	if [ "$(DISKTEST)" = "y" ]; then \
		sed -i "/RTCONFIG_DISKTEST/d" $(1); \
		echo "RTCONFIG_DISKTEST=y" >>$(1); \
	fi; \
	if [ "$(LOCALE2012)" = "y" ]; then \
		sed -i "/RTCONFIG_LOCALE2012/d" $(1); \
		echo "RTCONFIG_LOCALE2012=y" >>$(1); \
	fi; \
	if [ "$(ODMPID)" = "y" ]; then \
		sed -i "/RTCONFIG_ODMPID/d" $(1); \
		echo "RTCONFIG_ODMPID=y" >>$(1); \
	fi; \
	if [ "$(NONASUS)" = "y" ]; then \
		sed -i "/RTCONFIG_NONASUS/d" $(1); \
		echo "RTCONFIG_NONASUS=y" >>$(1); \
	fi; \
	if [ "$(MDNS)" = "y" ]; then \
		sed -i "/RTCONFIG_MDNS/d" $(1); \
		echo "RTCONFIG_MDNS=y" >>$(1); \
	fi; \
	if [ "$(REDIRECT_DNAME)" = "y" ]; then \
		sed -i "/RTCONFIG_REDIRECT_DNAME/d" $(1); \
		echo "RTCONFIG_REDIRECT_DNAME=y" >>$(1); \
	fi; \
	if [ "$(MTK_TW_AUTO_BAND4)" = "y" ]; then \
		sed -i "/RTCONFIG_MTK_TW_AUTO_BAND4/d" $(1); \
		echo "RTCONFIG_MTK_TW_AUTO_BAND4=y" >>$(1); \
	fi; \
	if [ "$(QCA_TW_AUTO_BAND4)" = "y" ]; then \
		sed -i "/RTCONFIG_QCA_TW_AUTO_BAND4/d" $(1); \
		echo "RTCONFIG_QCA_TW_AUTO_BAND4=y" >>$(1); \
	fi; \
	if [ "$(NEWSSID_REV2)" = "y" ]; then \
		sed -i "/RTCONFIG_NEWSSID_REV2/d" $(1); \
		echo "RTCONFIG_NEWSSID_REV2=y" >>$(1); \
	fi; \
	if [ "$(NEW_APP_ARM)" = "y" ]; then \
		sed -i "/RTCONFIG_NEW_APP_ARM/d" $(1); \
		echo "RTCONFIG_NEW_APP_ARM=y" >>$(1); \
	fi; \
	if [ "$(NEWSSID_REV2)" = "y" ]; then \
		sed -i "/RTCONFIG_NEWSSID_REV2/d" $(1); \
		echo "RTCONFIG_NEWSSID_REV2=y" >>$(1); \
	fi; \
	if [ "$(FINDASUS)" = "y" ]; then \
		sed -i "/RTCONFIG_FINDASUS/d" $(1); \
		echo "RTCONFIG_FINDASUS=y" >>$(1); \
		sed -i "/RTCONFIG_MDNS/d" $(1); \
		echo "RTCONFIG_MDNS=y" >>$(1); \
	fi; \
	if [ "$(TIMEMACHINE)" = "y" ]; then \
		sed -i "/RTCONFIG_TIMEMACHINE/d" $(1); \
		echo "RTCONFIG_TIMEMACHINE=y" >>$(1); \
		sed -i "/RTCONFIG_MDNS/d" $(1); \
		echo "RTCONFIG_MDNS=y" >>$(1); \
	fi; \
	if [ "$(LED_ALL)" = "y" ]; then \
		sed -i "/RTCONFIG_LED_ALL/d" $(1); \
		echo "RTCONFIG_LED_ALL=y" >>$(1); \
	fi; \
	if [ "$(N56U_SR2)" = "y" ]; then \
		sed -i "/RTCONFIG_N56U_SR2/d" $(1); \
		echo "RTCONFIG_N56U_SR2=y" >>$(1); \
	fi; \
	if [ "$(AP_CARRIER_DETECTION)" = "y" ]; then \
		sed -i "/RTCONFIG_AP_CARRIER_DETECTION/d" $(1); \
		echo "RTCONFIG_AP_CARRIER_DETECTION=y" >>$(1); \
	fi; \
	if [ "$(SFP)" = "y" ]; then \
		sed -i "/RTCONFIG_SFP/d" $(1); \
		echo "RTCONFIG_SFP=y" >>$(1); \
	fi; \
	if [ "$(SFP4M)" = "y" ]; then \
		sed -i "/RTCONFIG_SFP/d" $(1); \
		echo "RTCONFIG_SFP=y" >>$(1); \
		sed -i "/RTCONFIG_4M_SFP/d" $(1); \
		echo "RTCONFIG_4M_SFP=y" >>$(1); \
		sed -i "/RTCONFIG_UPNPC/d" $(1); \
		echo "# RTCONFIG_UPNPC is not set" >>$(1); \
		sed -i "/RTCONFIG_BONJOUR/d" $(1); \
		echo "# RTCONFIG_BONJOUR is not set" >>$(1); \
		sed -i "/RTCONFIG_SPEEDTEST/d" $(1); \
		echo "# RTCONFIG_SPEEDTEST is not set" >>$(1); \
	fi; \
	if [ "$(SFP8M)" = "y" ]; then \
		sed -i "/RTCONFIG_8M_SFP/d" $(1); \
		echo "RTCONFIG_8M_SFP=y" >>$(1); \
		sed -i "/RTCONFIG_UPNPC/d" $(1); \
		echo "# RTCONFIG_UPNPC is not set" >>$(1); \
		sed -i "/RTCONFIG_BONJOUR/d" $(1); \
		echo "# RTCONFIG_BONJOUR is not set" >>$(1); \
		sed -i "/RTCONFIG_SPEEDTEST/d" $(1); \
		echo "# RTCONFIG_SPEEDTEST is not set" >>$(1); \
	fi; \
	if [ "$(SFPRAM16M)" = "y" ]; then \
		sed -i "/RTCONFIG_16M_RAM_SFP/d" $(1); \
		echo "RTCONFIG_16M_RAM_SFP=y" >>$(1); \
	fi; \
	if [ "$(AUTODICT)" = "y" ]; then \
		sed -i "/RTCONFIG_AUTODICT/d" $(1); \
		echo "RTCONFIG_AUTODICT=y" >>$(1); \
	fi; \
	if [ "$(ZIPLIVEUPDATE)" = "y" ]; then \
		sed -i "/RTCONFIG_AUTOLIVEUPDATE_ZIP/d" $(1); \
		echo "RTCONFIG_AUTOLIVEUPDATE_ZIP=y" >>$(1); \
	fi; \
	if [ "$(LANWAN_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_LANWAN_LED/d" $(1); \
		echo "RTCONFIG_LANWAN_LED=y" >>$(1); \
	fi; \
	if [ "$(WLAN_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_WLAN_LED/d" $(1); \
		echo "RTCONFIG_WLAN_LED=y" >>$(1); \
	fi; \
	if [ "$(ETLAN_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_FAKE_ETLAN_LED/d" $(1); \
		echo "RTCONFIG_FAKE_ETLAN_LED=y" >>$(1); \
	fi; \
	if [ "$(EXT_LED_WPS)" = "y" ]; then \
		sed -i "/RTCONFIG_EXT_LED_WPS/d" $(1); \
		echo "RTCONFIG_EXT_LED_WPS=y" >>$(1); \
	fi; \
	if [ "$(LAN4WAN_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_LAN4WAN_LED/d" $(1); \
		echo "RTCONFIG_LAN4WAN_LED=y" >>$(1); \
	fi; \
	if [ "$(SWMODE_SWITCH)" = "y" ]; then \
		sed -i "/RTCONFIG_SWMODE_SWITCH/d" $(1); \
		echo "RTCONFIG_SWMODE_SWITCH=y" >>$(1); \
	fi; \
	if [ "$(WL_AUTO_CHANNEL)" = "y" ]; then \
		sed -i "/RTCONFIG_WL_AUTO_CHANNEL/d" $(1); \
		echo "RTCONFIG_WL_AUTO_CHANNEL=y" >>$(1); \
	fi; \
	if [ "$(SMALL_FW_UPDATE)" = "y" ]; then \
		sed -i "/RTCONFIG_SMALL_FW_UPDATE/d" $(1); \
		echo "RTCONFIG_SMALL_FW_UPDATE=y" >>$(1); \
	fi; \
	if [ "$(WIRELESS_SWITCH)" = "y" ]; then \
		sed -i "/RTCONFIG_WIRELESS_SWITCH/d" $(1); \
		echo "RTCONFIG_WIRELESS_SWITCH=y" >>$(1); \
	fi; \
	if [ "$(BTN_WIFITOG)" = "y" ]; then \
		sed -i "/RTCONFIG_WIFI_TOG_BTN/d" $(1); \
		echo "RTCONFIG_WIFI_TOG_BTN=y" >>$(1); \
	fi; \
	if [ "$(BTN_WPS_RST)" = "y" ]; then \
		sed -i "/RTCONFIG_WPS_RST_BTN/d" $(1); \
		echo "RTCONFIG_WPS_RST_BTN=y" >>$(1); \
	fi; \
	if [ "$(BTN_WPS_ALLLED)" = "y" ]; then \
		sed -i "/RTCONFIG_WPS_ALLLED_BTN/d" $(1); \
		echo "RTCONFIG_WPS_ALLLED_BTN=y" >>$(1); \
	fi; \
	if [ "$(LOGO_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_LOGO_LED/d" $(1); \
		echo "RTCONFIG_LOGO_LED=y" >>$(1); \
	fi; \
	if [ "$(LED_BTN)" = "y" ]; then \
		sed -i "/RTCONFIG_LED_BTN/d" $(1); \
		echo "RTCONFIG_LED_BTN=y" >>$(1); \
	fi; \
	if [ "$(USBEJECT)" = "y" ]; then \
		sed -i "/RTCONFIG_USBEJECT/d" $(1); \
		echo "RTCONFIG_USBEJECT=y" >>$(1); \
	fi; \
	if [ "$(BCM4352_5G)" = "y" ]; then \
		sed -i "/RTCONFIG_4352_5G/d" $(1); \
		echo "RTCONFIG_4352_5G=y" >>$(1); \
	fi; \
	if [ "$(ACCEL_PPTPD)" = "y" ]; then \
		sed -i "/RTCONFIG_ACCEL_PPTPD/d" $(1); \
		echo "RTCONFIG_ACCEL_PPTPD=y" >>$(1); \
	fi; \
	if [ "$(SNMPD)" = "y" ]; then \
		sed -i "/RTCONFIG_SNMPD/d" $(1); \
		echo "RTCONFIG_SNMPD=y" >>$(1); \
	fi; \
	if [ "$(SHP)" = "y" ]; then \
		sed -i "/RTCONFIG_SHP/d" $(1); \
		echo "RTCONFIG_SHP=y" >>$(1); \
	fi; \
	if [ "$(GRO)" = "y" ]; then \
		sed -i "/RTCONFIG_GROCTRL/d" $(1); \
		echo "RTCONFIG_GROCTRL=y" >>$(1); \
	fi; \
	if [ "$(DSL)" = "y" ]; then \
		sed -i "/RTCONFIG_DSL/d" $(1); \
		echo "RTCONFIG_DSL=y" >>$(1); \
		if [ "$(ANNEX_B)" = "y" ]; then \
			echo "RTCONFIG_DSL_ANNEX_B=y" >>$(1); \
		else \
			echo "# RTCONFIG_DSL_ANNEX_B is not set" >>$(1); \
		fi; \
		if [ "$(DSL_TCLINUX)" = "y" ]; then \
			sed -i "/RTCONFIG_DSL_TCLINUX/d" $(1); \
			echo "RTCONFIG_DSL_TCLINUX=y" >>$(1); \
		else \
			echo "# RTCONFIG_DSL_TCLINUX is not set" >>$(1); \
		fi; \
		if [ "$(VDSL)" = "y" ]; then \
			sed -i "/RTCONFIG_VDSL/d" $(1); \
			echo "RTCONFIG_VDSL=y" >>$(1); \
		else \
			echo "# RTCONFIG_VDSL is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(DUALWAN)" = "y" ]; then \
		sed -i "/RTCONFIG_DUALWAN/d" $(1); \
		echo "RTCONFIG_DUALWAN=y" >>$(1); \
	fi; \
	if [ "$(HW_DUALWAN)" = "y" ]; then \
		sed -i "/RTCONFIG_HW_DUALWAN/d" $(1); \
		echo "RTCONFIG_HW_DUALWAN=y" >>$(1); \
	fi; \
	if [ "$(EMAIL)" = "y" ]; then \
		sed -i "/RTCONFIG_PUSH_EMAIL/d" $(1); \
		echo "RTCONFIG_PUSH_EMAIL=y" >>$(1); \
	fi; \
	if [ "$(DBLOG)" = "y" ]; then \
		sed -i "/RTCONFIG_DBLOG/d" $(1); \
		echo "RTCONFIG_DBLOG=y" >>$(1); \
	fi; \
	if [ "$(SYSSTATE)" = "y" ]; then \
		sed -i "/RTCONFIG_SYSSTATE/d" $(1); \
		echo "RTCONFIG_SYSSTATE=y" >>$(1); \
	fi; \
	if [ "$(USER_LOW_RSSI)" = "y" ]; then \
		sed -i "/RTCONFIG_USER_LOW_RSSI/d" $(1); \
		echo "RTCONFIG_USER_LOW_RSSI=y" >>$(1); \
	fi; \
	if [ "$(ADV_RAST)" = "y" ]; then \
		sed -i "/RTCONFIG_ADV_RAST/d" $(1); \
		echo "RTCONFIG_ADV_RAST=y" >>$(1); \
	fi; \
	if [ "$(NEW_USER_LOW_RSSI)" = "y" ]; then \
		sed -i "/RTCONFIG_NEW_USER_LOW_RSSI/d" $(1); \
		echo "RTCONFIG_NEW_USER_LOW_RSSI=y" >>$(1); \
	fi; \
	if [ "$(USB)" = "USB" ]; then \
		sed -i "/RTCONFIG_USB\b/d" $(1); \
		echo "RTCONFIG_USB=y" >>$(1); \
		if [ "$(USBEXTRAS)" = "y" ]; then \
			sed -i "/RTCONFIG_USB_EXTRAS/d" $(1); \
			echo "RTCONFIG_USB_EXTRAS=y" >>$(1); \
		fi; \
		if [ "$(E2FSPROGS)" = "y" ]; then \
			sed -i "/RTCONFIG_E2FSPROGS/d" $(1); \
			echo "RTCONFIG_E2FSPROGS=y" >>$(1); \
		fi; \
		if [ "$(EXT4FS)" = "y" ]; then \
			sed -i "/RTCONFIG_EXT4FS/d" $(1); \
			echo "RTCONFIG_EXT4FS=y" >>$(1); \
		fi; \
		if [ "$(TFAT)" != "" ]; then \
			sed -i "/RTCONFIG_TFAT/d" $(1); \
			echo "RTCONFIG_TFAT=y" >>$(1); \
			if [ "$(findstring open, $(TFAT))" = "open" ]; then \
				sed -i "/RTCONFIG_OPENPLUS_TFAT/d" $(1); \
				echo "RTCONFIG_OPENPLUS_TFAT=y" >>$(1); \
			fi; \
		fi; \
		if [ "$(NTFS)" != "" ]; then \
			sed -i "/RTCONFIG_NTFS/d" $(1); \
			echo "RTCONFIG_NTFS=y" >>$(1); \
			if [ "$(findstring open, $(NTFS))" = "open" ]; then \
				sed -i "/RTCONFIG_OPEN_NTFS3G/d" $(1); \
				echo "RTCONFIG_OPEN_NTFS3G=y" >>$(1); \
				if [ "$(findstring paragon, $(NTFS))" = "paragon" ]; then \
					sed -i "/RTCONFIG_OPENPLUSPARAGON_NTFS/d" $(1); \
					echo "RTCONFIG_OPENPLUSPARAGON_NTFS=y" >>$(1); \
				elif [ "$(findstring tuxera, $(NTFS))" = "tuxera" ]; then \
					sed -i "/RTCONFIG_OPENPLUSTUXERA_NTFS/d" $(1); \
					echo "RTCONFIG_OPENPLUSTUXERA_NTFS=y" >>$(1); \
				fi; \
			fi; \
			if [ "$(findstring paragon, $(NTFS))" = "paragon" ]; then \
				sed -i "/RTCONFIG_PARAGON_NTFS/d" $(1); \
				echo "RTCONFIG_PARAGON_NTFS=y" >>$(1); \
			fi; \
			if [ "$(findstring tuxera, $(NTFS))" = "tuxera" ]; then \
				sed -i "/RTCONFIG_TUXERA_NTFS/d" $(1); \
				echo "RTCONFIG_TUXERA_NTFS=y" >>$(1); \
			fi; \
		fi; \
		if [ "$(HFS)" != "" ]; then \
			sed -i "/RTCONFIG_HFS/d" $(1); \
			echo "RTCONFIG_HFS=y" >>$(1); \
			if [ "$(findstring open, $(HFS))" = "open" ]; then \
				sed -i "/RTCONFIG_KERNEL_HFSPLUS/d" $(1); \
				echo "RTCONFIG_KERNEL_HFSPLUS=y" >>$(1); \
				if [ "$(findstring paragon, $(HFS))" = "paragon" ]; then \
					sed -i "/RTCONFIG_OPENPLUSPARAGON_HFS/d" $(1); \
					echo "RTCONFIG_OPENPLUSPARAGON_HFS=y" >>$(1); \
				elif [ "$(findstring tuxera, $(HFS))" = "tuxera" ]; then \
					sed -i "/RTCONFIG_OPENPLUSTUXERA_HFS/d" $(1); \
					echo "RTCONFIG_OPENPLUSTUXERA_HFS=y" >>$(1); \
				fi; \
			fi; \
			if [ "$(findstring paragon, $(HFS))" = "paragon" ]; then \
				sed -i "/RTCONFIG_PARAGON_HFS/d" $(1); \
				echo "RTCONFIG_PARAGON_HFS=y" >>$(1); \
			fi; \
			if [ "$(findstring tuxera, $(HFS))" = "tuxera" ]; then \
				sed -i "/RTCONFIG_TUXERA_HFS/d" $(1); \
				echo "RTCONFIG_TUXERA_HFS=y" >>$(1); \
			fi; \
		fi; \
		if [ "$(UFSDDEBUG)" = "y" ]; then \
			sed -i "/RTCONFIG_UFSD_DEBUG/d" $(1); \
			echo "RTCONFIG_UFSD_DEBUG=y" >>$(1); \
		fi; \
		if [ "$(DISK_MONITOR)" = "y" ]; then \
			sed -i "/RTCONFIG_DISK_MONITOR/d" $(1); \
			echo "RTCONFIG_DISK_MONITOR=y" >>$(1); \
		fi; \
		if [ "$(MEDIASRV)" = "y" ]; then \
			sed -i "/RTCONFIG_MEDIA_SERVER/d" $(1); \
			echo "RTCONFIG_MEDIA_SERVER=y" >>$(1); \
			if [ "$(MEDIASRV_LIMIT)" = "y" ]; then \
			sed -i "/RTCONFIG_MEDIASERVER_LIMIT/d" $(1); \
			echo "RTCONFIG_MEDIASERVER_LIMIT=y" >>$(1); \
			fi; \
			sed -i "/RTCONFIG_NO_DAAPD/d" $(1); \
			if [ "$(NO_DAAPD)" = "y" ]; then \
				echo "RTCONFIG_NO_DAAPD=y" >>$(1); \
			else \
				echo "# RTCONFIG_NO_DAAPD is not set" >>$(1); \
			fi; \
		fi; \
		if [ "$(SMARTSYNCBASE)" = "y" ]; then \
				sed -i "/RTCONFIG_SWEBDAVCLIENT/d" $(1); \
				echo "RTCONFIG_SWEBDAVCLIENT=y" >>$(1); \
				sed -i "/RTCONFIG_DROPBOXCLIENT/d" $(1); \
				echo "RTCONFIG_DROPBOXCLIENT=y" >>$(1); \
				sed -i "/RTCONFIG_GOOGLECLIENT/d" $(1); \
				echo "RTCONFIG_GOOGLECLIENT=y" >>$(1); \
				sed -i "/RTCONFIG_FTPCLIENT/d" $(1); \
				echo "RTCONFIG_FTPCLIENT=y" >>$(1); \
				sed -i "/RTCONFIG_SAMBACLIENT/d" $(1); \
				echo "RTCONFIG_SAMBACLIENT=y" >>$(1); \
				sed -i "/RTCONFIG_USBCLIENT/d" $(1); \
				echo "RTCONFIG_USBCLIENT=y" >>$(1); \
				sed -i "/RTCONFIG_CLOUDSYNC/d" $(1); \
				echo "RTCONFIG_CLOUDSYNC=y" >>$(1); \
		else \
			if [ "$(SWEBDAVCLIENT)" = "y" ]; then \
				sed -i "/RTCONFIG_SWEBDAVCLIENT/d" $(1); \
				echo "RTCONFIG_SWEBDAVCLIENT=y" >>$(1); \
			fi; \
			if [ "$(DROPBOXCLIENT)" = "y" ]; then \
				sed -i "/RTCONFIG_DROPBOXCLIENT/d" $(1); \
				echo "RTCONFIG_DROPBOXCLIENT=y" >>$(1); \
			fi; \
			if [ "$(GOOGLECLIENT)" = "y" ]; then \
				sed -i "/RTCONFIG_GOOGLECLIENT/d" $(1); \
				echo "RTCONFIG_GOOGLECLIENT=y" >>$(1); \
			fi; \
			if [ "$(FTPCLIENT)" = "y" ]; then \
				sed -i "/RTCONFIG_FTPCLIENT/d" $(1); \
				echo "RTCONFIG_FTPCLIENT=y" >>$(1); \
			fi; \
			if [ "$(SAMBACLIENT)" = "y" ]; then \
				sed -i "/RTCONFIG_SAMBACLIENT/d" $(1); \
				echo "RTCONFIG_SAMBACLIENT=y" >>$(1); \
			fi; \
			if [ "$(USBCLIENT)" = "y" ]; then \
				sed -i "/RTCONFIG_USBCLIENT/d" $(1); \
				echo "RTCONFIG_USBCLIENT=y" >>$(1); \
			fi; \
			if [ "$(FLICKRCLIENT)" = "y" ]; then \
				sed -i "/RTCONFIG_FLICKRCLIENT/d" $(1); \
				echo "RTCONFIG_FLICKRCLIENT=y" >>$(1); \
			fi; \
			if [ "$(CLOUDSYNC)" = "y" ]; then \
				sed -i "/RTCONFIG_CLOUDSYNC/d" $(1); \
				echo "RTCONFIG_CLOUDSYNC=y" >>$(1); \
			fi; \
		fi; \
		if [ "$(MODEM)" = "y" ]; then \
			sed -i "/RTCONFIG_USB_MODEM/d" $(1); \
			echo "RTCONFIG_USB_MODEM=y" >>$(1); \
			if [ "$(MODEMPIN)" = "n" ]; then \
				echo "# RTCONFIG_USB_MODEM_PIN is not set" >>$(1); \
			else \
				echo "RTCONFIG_USB_MODEM_PIN=y" >>$(1); \
			fi; \
			if [ "$(BECEEM)" = "y" ]; then \
				sed -i "/RTCONFIG_USB_BECEEM/d" $(1); \
				echo "RTCONFIG_USB_BECEEM=y" >>$(1); \
			fi; \
			if [ "$(GOBI)" = "y" ]; then \
				sed -i "/RTCONFIG_INTERNAL_GOBI/d" $(1); \
				echo "RTCONFIG_INTERNAL_GOBI=y" >>$(1); \
			fi; \
			if [ "$(LESSMODEM)" = "y" ]; then \
				sed -i "/RTCONFIG_USB_LESSMODEM/d" $(1); \
				echo "RTCONFIG_USB_LESSMODEM=y" >>$(1); \
			fi; \
			if [ "$(DYNMODEM)" = "y" ]; then \
				sed -i "/RTCONFIG_DYN_MODEM/d" $(1); \
				echo "RTCONFIG_DYN_MODEM=y" >>$(1); \
			fi; \
			if [ "$(USBSMS)" = "y" ]; then \
				sed -i "/RTCONFIG_USB_SMS_MODEM/d" $(1); \
				echo "RTCONFIG_USB_SMS_MODEM=y" >>$(1); \
			fi; \
			if [ "$(MULTIMODEM)" = "y" ]; then \
				sed -i "/RTCONFIG_USB_MULTIMODEM/d" $(1); \
				echo "RTCONFIG_USB_MULTIMODEM=y" >>$(1); \
			fi; \
			if [ "$(MODEMBRIDGE)" = "y" ]; then \
				sed -i "/RTCONFIG_MODEM_BRIDGE/d" $(1); \
				echo "RTCONFIG_MODEM_BRIDGE=y" >>$(1); \
			fi; \
		fi; \
		if [ "$(PRINTER)" = "y" ]; then \
			sed -i "/RTCONFIG_USB_PRINTER/d" $(1); \
			echo "RTCONFIG_USB_PRINTER=y" >>$(1); \
		fi; \
		if [ "$(WEBDAV)" = "y" ]; then \
			sed -i "/RTCONFIG_WEBDAV/d" $(1); \
			echo "RTCONFIG_WEBDAV=y" >>$(1); \
		fi; \
		if [ "$(USBAP)" = "y" ]; then \
			sed -i "/RTCONFIG_BRCM_USBAP/d" $(1); \
			echo "RTCONFIG_BRCM_USBAP=y" >>$(1); \
			if [ "$(BUILD_NAME)" != "RT-AC53U" ]; then \
				sed -i "/EPI_VERSION_NUM/d" include/epivers.h; \
				sed -i "/#endif \/\* _epivers_h_ \*\//d" include/epivers.h; \
				echo "#define	EPI_VERSION_NUM		$(DONGLE_VER)" >>include/epivers.h; \
				echo "#endif /* _epivers_h_ */" >>include/epivers.h; \
			fi; \
		fi; \
		if [ "$(XHCI)" = "y" ]; then \
			sed -i "/RTCONFIG_USB_XHCI/d" $(1); \
			echo "RTCONFIG_USB_XHCI=y" >>$(1); \
		fi; \
		if [ "$(NFS)" = "y" ]; then \
			sed -i "/RTCONFIG_NFS/d" $(1); \
			echo "RTCONFIG_NFS=y" >>$(1); \
		fi; \
	else \
		sed -i "/RTCONFIG_USB\b/d" $(1); \
		echo "# RTCONFIG_USB is not set" >>$(1); \
	fi; \
	if [ "$(HTTPS)" = "y" ]; then \
		sed -i "/RTCONFIG_HTTPS/d" $(1); \
		echo "RTCONFIG_HTTPS=y" >>$(1); \
	fi; \
	if [ "$(USBRESET)" = "y" ]; then \
		sed -i "/RTCONFIG_USBRESET/d" $(1); \
		echo "RTCONFIG_USBRESET=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_USBRESET/d" $(1); \
		echo "# RTCONFIG_USBRESET is not set" >>$(1); \
	fi; \
	if [ "$(WIFIPWR)" = "y" ]; then \
		sed -i "/RTCONFIG_WIFIPWR/d" $(1); \
		echo "RTCONFIG_WIFIPWR=y" >>$(1); \
	fi; \
	if [ "$(XHCIMODE)" = "y" ]; then \
		sed -i "/RTCONFIG_XHCIMODE/d" $(1); \
		echo "RTCONFIG_XHCIMODE=y" >>$(1); \
	fi; \
	if [ "$(NO_SAMBA)" = "y" ]; then \
		sed -i "/RTCONFIG_SAMBASRV/d" $(1); \
		echo "# RTCONFIG_SAMBASRV is not set" >>$(1); \
	fi; \
	if [ "$(NO_FTP)" = "y" ]; then \
		sed -i "/RTCONFIG_FTP/d" $(1); \
		echo "# RTCONFIG_FTP is not set" >>$(1); \
	fi; \
	if [ "$(NO_USBSTORAGE)" = "y" ]; then \
		sed -i "/RTCONFIG_NO_USBPORT/d" $(1); \
		echo "RTCONFIG_NO_USBPORT=y" >>$(1); \
		sed -i "/RTCONFIG_SAMBASRV/d" $(1); \
		echo "# RTCONFIG_SAMBASRV is not set" >>$(1); \
		sed -i "/RTCONFIG_FTP/d" $(1); \
		echo "# RTCONFIG_FTP is not set" >>$(1); \
	fi; \
	if [ "$(ZEBRA)" = "y" ]; then \
		sed -i "/RTCONFIG_ZEBRA/d" $(1); \
		echo "RTCONFIG_ZEBRA=y" >>$(1); \
	fi; \
	if [ "$(JFFS2)" = "y" ]; then \
		sed -i "/RTCONFIG_JFFS2/d" $(1); \
		echo "RTCONFIG_JFFS2=y" >>$(1); \
	fi; \
	if [ "$(BRCM_NAND_JFFS2)" = "y" ]; then \
		sed -i "/RTCONFIG_BRCM_NAND_JFFS2/d" $(1); \
		echo "RTCONFIG_BRCM_NAND_JFFS2=y" >>$(1); \
	fi; \
	if [ "$(JFFS_NVRAM)" = "y" ]; then \
		sed -i "/RTCONFIG_JFFS_NVRAM/d" $(1); \
		echo "RTCONFIG_JFFS_NVRAM=y" >>$(1); \
	fi; \
	if [ "$(JFFS1)" = "y" ]; then \
		sed -i "/RTCONFIG_JFFSV1/d" $(1); \
		echo "RTCONFIG_JFFSV1=y" >>$(1); \
	fi; \
	if [ "$(CIFS)" = "y" ]; then \
		sed -i "/RTCONFIG_CIFS/d" $(1); \
		echo "RTCONFIG_CIFS=y" >>$(1); \
		sed -i "/RTCONFIG_AUTODICT/d" $(1); \
		echo "RTCONFIG_AUTODICT=n" >>$(1); \
	fi; \
	if [ "$(SSH)" = "y" ]; then \
		sed -i "/RTCONFIG_SSH/d" $(1); \
		echo "RTCONFIG_SSH=y" >>$(1); \
	fi; \
	if [ "$(NO_LIBOPT)" = "y" ]; then \
		sed -i "/RTCONFIG_OPTIMIZE_SHARED_LIBS/d" $(1); \
		echo "# RTCONFIG_OPTIMIZE_SHARED_LIBS is not set" >>$(1); \
	fi; \
	if [ "$(EBTABLES)" = "y" ]; then \
		sed -i "/RTCONFIG_EBTABLES/d" $(1); \
		echo "RTCONFIG_EBTABLES=y" >>$(1); \
	fi; \
	if [ "$(IPV6SUPP)" = "y" ]; then \
		sed -i "/RTCONFIG_IPV6/d" $(1); \
		echo "RTCONFIG_IPV6=y" >>$(1); \
	fi; \
	if [ "$(IPSEC)" = "y" ] || \
	   [ "$(IPSEC)" = "QUICKSEC" ] || \
	   [ "$(IPSEC)" = "STRONGSWAN" ] ; then \
		sed -i "/RTCONFIG_IPSEC/d" $(1); \
		echo "RTCONFIG_IPSEC=y" >>$(1); \
		for ipsec in $(IPSEC_ID_POOL) ; do \
			sed -i "/RTCONFIG_$${ipsec}\>/d" $(1); \
			if [ "$(IPSEC)" = "$${ipsec}" ] ; then \
				echo "RTCONFIG_$${ipsec}=y" >> $(1); \
				if [ "$(IPSEC_SRVCLI_ONLY)" = "SRV" ]; then \
					sed -i "/RTCONFIG_IPSEC_SERVER/d" $(1); \
					echo "RTCONFIG_IPSEC_SERVER=y" >>$(1); \
					echo "# RTCONFIG_IPSEC_CLIENT is not set" >>$(1); \
				elif [ "$(IPSEC_SRVCLI_ONLY)" = "CLI" ]; then \
					sed -i "/RTCONFIG_IPSEC_CLIENT/d" $(1); \
					echo "RTCONFIG_IPSEC_CLIENT=y" >>$(1); \
					echo "# RTCONFIG_IPSEC_SERVER is not set" >>$(1); \
				else \
					echo "RTCONFIG_IPSEC_SERVER=y" >>$(1); \
					echo "RTCONFIG_IPSEC_CLIENT=y" >>$(1); \
				fi; \
			elif [ "$(IPSEC)" = "y" -a "$${ipsec}" = "STRONGSWAN" ] ; then \
				sed -i "/RTCONFIG_STRONGSWAN/d" $(1); \
				echo "RTCONFIG_STRONGSWAN=y" >>$(1); \
			else \
				echo "# RTCONFIG_$${ipsec} is not set" >> $(1); \
			fi; \
		done; \
	else \
		sed -i "/RTCONFIG_IPSEC/d" $(1); \
		echo "# RTCONFIG_IPSEC is not set" >>$(1); \
		for ipsec in $(IPSEC_ID_POOL) ; do \
			sed -i "/RTCONFIG_$${ipsec}\>/d" $(1); \
			echo "# RTCONFIG_$${ipsec} is not set" >> $(1); \
		done; \
		echo "# RTCONFIG_IPSEC_SERVER is not set" >>$(1); \
		echo "# RTCONFIG_IPSEC_CLIENT is not set" >>$(1); \
	fi; \
	if [ "$(OPENVPN)" = "y" ]; then \
		sed -i "/RTCONFIG_LZO/d" $(1); \
		echo "RTCONFIG_LZO=y" >>$(1); \
		sed -i "/RTCONFIG_OPENVPN/d" $(1); \
		echo "RTCONFIG_OPENVPN=y" >>$(1); \
 	fi; \
	if [ "$(APP)" = "installed" ]; then \
		sed -i "/RTCONFIG_APP_PREINSTALLED/d" $(1); \
		echo "RTCONFIG_APP_PREINSTALLED=y" >>$(1); \
	elif [ "$(APP)" = "network" ]; then \
		sed -i "/RTCONFIG_APP_NETINSTALLED/d" $(1); \
		echo "RTCONFIG_APP_NETINSTALLED=y" >>$(1); \
	fi; \
	if [ "$(STRACE)" = "y" ] ; then \
		sed -i "/RTCONFIG_STRACE/d" $(1); \
		echo "RTCONFIG_STRACE=y" >>$(1); \
	fi; \
	if [ "$(ISP_METER)" = "y" ]; then \
		sed -i "/RTCONFIG_ISP_METER/d" $(1); \
		echo "RTCONFIG_ISP_METER=y" >>$(1); \
	fi; \
	if [ "$(NVRAM_64K)" = "y" ]; then \
		sed -i "/RTCONFIG_NVRAM_64K/d" $(1); \
		echo "RTCONFIG_NVRAM_64K=y" >>$(1); \
	fi; \
	if [ "$(DUAL_TRX)" = "y" ]; then \
		sed -i "/RTCONFIG_DUAL_TRX/d" $(1); \
		echo "RTCONFIG_DUAL_TRX=y" >>$(1); \
	fi; \
	if [ "$(PSISTLOG)" = "y" ]; then \
		sed -i "/RTCONFIG_PSISTLOG/d" $(1); \
		echo "RTCONFIG_PSISTLOG=y" >>$(1); \
	fi; \
	if [ "$(UBI)" = "y" ]; then \
		sed -i "/RTCONFIG_UBI/d" $(1); \
		echo "RTCONFIG_UBI=y" >>$(1); \
		if [ "$(UBIFS)" = "y" ]; then \
			sed -i "/RTCONFIG_UBIFS/d" $(1); \
			echo "RTCONFIG_UBIFS=y" >>$(1); \
			sed -i "/RTCONFIG_JFFS2/d" $(1); \
			echo "# RTCONFIG_JFFS2 is not set" >>$(1); \
			sed -i "/RTCONFIG_JFFSV1/d" $(1); \
			echo "# RTCONFIG_JFFSV1 is not set" >>$(1); \
			sed -i "/RTCONFIG_JFFS2USERICON/d" $(1); \
			echo "RTCONFIG_JFFS2USERICON=y" >>$(1); \
		else \
			sed -i "/RTCONFIG_UBIFS/d" $(1); \
			echo "# RTCONFIG_UBIFS is not set" >>$(1); \
		fi; \
	else \
		sed -i "/RTCONFIG_UBI/d" $(1); \
		echo "# RTCONFIG_UBI is not set" >>$(1); \
		sed -i "/RTCONFIG_UBIFS/d" $(1); \
		echo "# RTCONFIG_UBIFS is not set" >>$(1); \
	fi; \
	if [ "$(UBI)" = "y" ] || [ "$(JFFS2)" = "y" ] ; then \
		if [ "$(SAVEJFFS)" = "y" ] ; then \
			sed -i "/RTCONFIG_SAVEJFFS/d" $(1); \
			echo "RTCONFIG_SAVEJFFS=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(OPTIMIZE_XBOX)" = "y" ]; then \
		sed -i "/RTCONFIG_OPTIMIZE_XBOX/d" $(1); \
		echo "RTCONFIG_OPTIMIZE_XBOX=y" >>$(1); \
	fi; \
	if [ "$(NEW_RGDM)" = "y" ]; then \
		sed -i "/RTCONFIG_NEW_REGULATION_DOMAIN/d" $(1); \
		echo "RTCONFIG_NEW_REGULATION_DOMAIN=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_NEW_REGULATION_DOMAIN/d" $(1); \
		echo "# RTCONFIG_NEW_REGULATION_DOMAIN is not set" >>$(1); \
	fi; \
	if [ "$(DYN_DICT_NAME)" = "y" ]; then \
		sed -i "/RTCONFIG_DYN_DICT_NAME/d" $(1); \
		echo "RTCONFIG_DYN_DICT_NAME=y" >>$(1); \
	fi; \
	if [ "$(DMALLOC)" = "y" ]; then \
		sed -i "/RTCONFIG_DMALLOC/d" $(1); \
		echo "RTCONFIG_DMALLOC=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_DMALLOC/d" $(1); \
		echo "# RTCONFIG_DMALLOC is not set" >>$(1); \
	fi; \
	if [ "$(JFFS2ND_BACKUP)" = "y" ]; then \
		sed -i "/RTCONFIG_JFFS2ND_BACKUP/d" $(1); \
		echo "RTCONFIG_JFFS2ND_BACKUP=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_JFFS2ND_BACKUP/d" $(1); \
		echo "# RTCONFIG_JFFS2ND_BACKUP is not set" >>$(1); \
	fi; \
	if [ "$(TEMPROOTFS)" = "y" ]; then \
		sed -i "/RTCONFIG_TEMPROOTFS/d" $(1); \
		echo "RTCONFIG_TEMPROOTFS=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_TEMPROOTFS/d" $(1); \
		echo "# RTCONFIG_TEMPROOTFS is not set" >>$(1); \
	fi; \
	if [ "$(ATEUSB3_FORCE)" = "y" ]; then \
		sed -i "/RTCONFIG_ATEUSB3_FORCE/d" $(1); \
		echo "RTCONFIG_ATEUSB3_FORCE=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_ATEUSB3_FORCE/d" $(1); \
		echo "# RTCONFIG_ATEUSB3_FORCE is not set" >>$(1); \
	fi; \
	if [ "$(JFFS2LOG)" = "y" ]; then \
		sed -i "/RTCONFIG_JFFS2LOG/d" $(1); \
		echo "RTCONFIG_JFFS2LOG=y" >>$(1); \
		sed -i "/RTCONFIG_JFFS2USERICON/d" $(1); \
		echo "RTCONFIG_JFFS2USERICON=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_JFFS2LOG/d" $(1); \
		echo "# RTCONFIG_JFFS2LOG is not set" >>$(1); \
		if [ "$(UBIFS)" = "y" ]; then \
			sed -i "/RTCONFIG_JFFS2USERICON/d" $(1); \
			echo "RTCONFIG_JFFS2USERICON=y" >>$(1); \
		else \
			sed -i "/RTCONFIG_JFFS2USERICON/d" $(1); \
			echo "# RTCONFIG_JFFS2USERICON is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(WPSMULTIBAND)" = "y" ]; then \
		sed -i "/RTCONFIG_WPSMULTIBAND/d" $(1); \
		echo "RTCONFIG_WPSMULTIBAND=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_WPSMULTIBAND/d" $(1); \
		echo "# RTCONFIG_WPSMULTIBAND is not set" >>$(1); \
	fi; \
	if [ "$(RALINK_DFS)" = "y" ]; then \
		sed -i "/RTCONFIG_RALINK_DFS/d" $(1); \
		echo "RTCONFIG_RALINK_DFS=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_RALINK_DFS/d" $(1); \
		echo "# RTCONFIG_RALINK_DFS is not set" >>$(1); \
	fi; \
	if [ "$(EM)" = "y" ]; then \
		sed -i "/RTCONFIG_ENGINEERING_MODE/d" $(1); \
		echo "RTCONFIG_ENGINEERING_MODE=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_ENGINEERING_MODE/d" $(1); \
		echo "# RTCONFIG_ENGINEERING_MODE is not set" >>$(1); \
	fi; \
	if [ "$(VPNC)" = "y" ]; then \
		sed -i "/RTCONFIG_VPNC/d" $(1); \
		echo "RTCONFIG_VPNC=y" >>$(1); \
	fi; \
	if [ "$(KYIVSTAR)" = "y" ]; then \
		sed -i "/RTCONFIG_KYIVSTAR/d" $(1); \
		echo "RTCONFIG_KYIVSTAR=y" >>$(1); \
	fi; \
	if [ "$(TFTPSRV)" = "y" ]; then \
		sed -i "/RTCONFIG_TFTP_SERVER/d" $(1); \
		echo "RTCONFIG_TFTP_SERVER=y" >>$(1); \
	fi; \
	if [ "$(ETRON_XHCI)" = "y" ]; then \
		sed -i "/RTCONFIG_ETRON_XHCI\>/d" $(1); \
		echo "RTCONFIG_ETRON_XHCI=y" >>$(1); \
		if [ "$(ETRON_XHCI_USB3_LED)" = "y" ]; then \
			sed -i "/RTCONFIG_ETRON_XHCI_USB3_LED/d" $(1); \
			echo "RTCONFIG_ETRON_XHCI_USB3_LED=y" >>$(1); \
		else \
			sed -i "/RTCONFIG_ETRON_XHCI_USB3_LED/d" $(1); \
			echo "# RTCONFIG_ETRON_XHCI_USB3_LED is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(WANPORT2)" = "y" ]; then \
		sed -i "/RTCONFIG_WANPORT2/d" $(1); \
		echo "RTCONFIG_WANPORT2=y" >>$(1); \
	fi; \
	if [ "$(MTWANCFG)" = "y" ]; then \
		sed -i "/RTCONFIG_MULTIWAN_CFG/d" $(1); \
		echo "RTCONFIG_MULTIWAN_CFG=y" >>$(1); \
	fi; \
	if [ "$(WPS_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_WPS_LED/d" $(1); \
		echo "RTCONFIG_WPS_LED=y" >>$(1); \
	fi; \
	if [ "$(WANRED_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_WANRED_LED/d" $(1); \
		echo "RTCONFIG_WANRED_LED=y" >>$(1); \
	fi; \
	if [ "$(PWRRED_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_PWRRED_LED/d" $(1); \
		echo "RTCONFIG_PWRRED_LED=y" >>$(1); \
	fi; \
	if [ "$(FO_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_FAILOVER_LED/d" $(1); \
		echo "RTCONFIG_FAILOVER_LED=y" >>$(1); \
	fi; \
	if [ "$(BLINK_LED)" = "y" ]; then \
		sed -i "/RTCONFIG_BLINK_LED/d" $(1); \
		echo "RTCONFIG_BLINK_LED=y" >>$(1); \
	fi; \
	if [ "$(EJUSB_BTN)" = "y" ]; then \
		sed -i "/RTCONFIG_EJUSB_BTN/d" $(1); \
		echo "RTCONFIG_EJUSB_BTN=y" >>$(1); \
	fi; \
	if [ "$(M2_SSD)" = "y" ]; then \
		sed -i "/RTCONFIG_M2_SSD/d" $(1); \
		echo "RTCONFIG_M2_SSD=y" >>$(1); \
	fi; \
	if [ "$(WIGIG)" = "y" ]; then \
		sed -i "/RTCONFIG_WIGIG/d" $(1); \
		echo "RTCONFIG_WIGIG=y" >>$(1); \
	fi; \
	if [ "$(ATF)" = "y" ]; then \
		sed -i "/RTCONFIG_AIR_TIME_FAIRNESS/d" $(1); \
		echo "RTCONFIG_AIR_TIME_FAIRNESS=y" >>$(1); \
	fi; \
	if [ "$(PWRSAVE)" = "y" ]; then \
		sed -i "/RTCONFIG_POWER_SAVE/d" $(1); \
		echo "RTCONFIG_POWER_SAVE=y" >>$(1); \
	fi; \
	if [ "$(CFE_NVRAM_CHK)" = "y" ]; then \
		sed -i "/RTCONFIG_CFE_NVRAM_CHK/d" $(1); \
		echo "RTCONFIG_CFE_NVRAM_CHK=y" >>$(1); \
	fi; \
	if [ "$(LLDP)" = "y" ]; then \
		sed -i "/RTCONFIG_LLDP/d" $(1); \
		echo "RTCONFIG_LLDP=y" >>$(1); \
	fi; \
	if [ "$(DEBUG)" = "y" ]; then \
		sed -i "/RTCONFIG_DEBUG/d" $(1); \
		echo "RTCONFIG_DEBUG=y" >>$(1); \
		sed -i "/RTCONFIG_GDB/d" $(1); \
		echo "RTCONFIG_GDB=y" >>$(1); \
	fi; \
	if [ "$(UIDEBUG)" = "y" ]; then \
		sed -i "/RTCONFIG_UIDEBUG/d" $(1); \
		echo "RTCONFIG_UIDEBUG=y" >>$(1); \
		sed -i "/RTCONFIG_CIFS/d" $(1); \
		echo "RTCONFIG_CIFS=y" >>$(1); \
		sed -i "/RTCONFIG_AUTODICT/d" $(1); \
		echo "# RTCONFIG_AUTODICT is not set" >>$(1); \
	fi; \
	if [ "$(ROG)" = "y" ]; then \
		sed -i "/RTCONFIG_ROG/d" $(1); \
		echo "RTCONFIG_ROG=y" >>$(1); \
	fi; \
	if [ "$(GEOIP)" = "y" ]; then \
		sed -i "/RTCONFIG_GEOIP/d" $(1); \
		echo "RTCONFIG_GEOIP=y" >>$(1); \
	fi; \
	if [ "$(TRANSMISSION)" = "y" ]; then \
		sed -i "/RTCONFIG_TRANSMISSION/d" $(1); \
		echo "RTCONFIG_TRANSMISSION=y" >>$(1); \
	fi; \
	if [ "$(SINGLE_2G)" = "y" ]; then \
		sed -i "/RTCONFIG_HAS_5G\>/d" $(1); \
		echo "# RTCONFIG_HAS_5G is not set" >>$(1); \
	fi; \
	if [ "$(HAS_5G_2)" = "y" ]; then \
		sed -i "/RTCONFIG_HAS_5G_2\>/d" $(1); \
		echo "RTCONFIG_HAS_5G_2=y" >>$(1); \
	fi; \
	if [ "$(TFTP)" = "y" ]; then \
		sed -i "/RTCONFIG_TFTP\>/d" $(1); \
		echo "RTCONFIG_TFTP=y" >>$(1); \
	fi; \
	if [ "$(QTN)" = "y" ]; then \
		sed -i "/RTCONFIG_QTN/d" $(1); \
		echo "RTCONFIG_QTN=y" >>$(1); \
	fi; \
	if [ "$(LACP)" = "y" ]; then \
		sed -i "/RTCONFIG_LACP/d" $(1); \
		echo "RTCONFIG_LACP=y" >>$(1); \
	fi; \
	if [ "$(BCM_RECVFILE)" = "y" ] && [ "$(HND_ROUTER)" != "y" ]; then \
		sed -i "/RTCONFIG_RECVFILE/d" $(1); \
		echo "RTCONFIG_RECVFILE=y" >>$(1); \
	fi; \
	if [ "$(RGMII_BRCM5301X)" = "y" ]; then \
		sed -i "/RTCONFIG_RGMII_BRCM5301X/d" $(1); \
		echo "RTCONFIG_RGMII_BRCM5301X=y" >>$(1); \
	fi; \
	if [ "$(WPS_DUALBAND)" = "y" ]; then \
		sed -i "/RTCONFIG_WPS_DUALBAND/d" $(1); \
		echo "RTCONFIG_WPS_DUALBAND=y" >>$(1); \
	fi; \
	if [ "$(WIFICLONE)" = "y" ]; then \
		sed -i "/RTCONFIG_WPS_ENROLLEE/d" $(1); \
		echo "RTCONFIG_WPS_ENROLLEE=y" >>$(1); \
		sed -i "/RTCONFIG_WIFI_CLONE/d" $(1); \
		echo "RTCONFIG_WIFI_CLONE=y" >>$(1); \
	fi; \
	if [ "$(N18UTXBF)" = "y" ]; then \
		sed -i "/RTCONFIG_N18UTXBF/d" $(1); \
		echo "RTCONFIG_N18UTXBF=y" >>$(1); \
	fi; \
	if [ "$(BWDPI)" = "y" ]; then \
		sed -i "/RTCONFIG_BWDPI\>/d" $(1); \
		echo "RTCONFIG_BWDPI=y" >>$(1); \
		sed -i "/RTCONFIG_NOTIFICATION_CENTER/d" $(1); \
		echo "RTCONFIG_NOTIFICATION_CENTER=y" >>$(1); \
	fi; \
	if [ "$(NOTIFICATION_CENTER)" = "y" ]; then \
		sed -i "/RTCONFIG_NOTIFICATION_CENTER/d" $(1); \
		echo "RTCONFIG_NOTIFICATION_CENTER=y" >>$(1); \
	fi; \
	if [ "$(PROTECTION_SERVER)" = "y" ]; then \
		sed -i "/RTCONFIG_PROTECTION_SERVER/d" $(1); \
		echo "RTCONFIG_PROTECTION_SERVER=y" >>$(1); \
	fi; \
	if [ "$(WLCEVENTD)" = "y" ] || [ "$(CONFIG_BCMWL5)" = "y" ]; then \
		sed -i "/RTCONFIG_WLCEVENTD/d" $(1); \
		echo "RTCONFIG_WLCEVENTD=y" >>$(1); \
	fi; \
	if [ "$(FBT)" = "y" ]; then \
		sed -i "/RTCONFIG_FBT/d" $(1); \
		echo "RTCONFIG_FBT=y" >>$(1); \
	fi; \
	if [ "$(HAPDEVENT)" = "y" ] || [ "$(CONFIG_LANTIQ)" = "y" ]; then \
		sed -i "/RTCONFIG_HAPDEVENT/d" $(1); \
		echo "RTCONFIG_HAPDEVENT=y" >>$(1); \
	fi; \
	if [ "$(ADBLOCK)" = "y" ]; then \
		sed -i "/RTCONFIG_ADBLOCK/d" $(1); \
		echo "RTCONFIG_ADBLOCK=y" >>$(1); \
	fi; \
	if [ "$(TRAFFIC_LIMITER)" = "y" ]; then \
		sed -i "/RTCONFIG_TRAFFIC_LIMITER/d" $(1); \
		echo "RTCONFIG_TRAFFIC_LIMITER=y" >>$(1); \
		sed -i "/RTCONFIG_NOTIFICATION_CENTER/d" $(1); \
		echo "RTCONFIG_NOTIFICATION_CENTER=y" >>$(1); \
	fi; \
	if [ "$(BCM5301X_TRAFFIC_MONITOR)" = "y" ]; then \
		sed -i "/RTCONFIG_BCM5301X_TRAFFIC_MONITOR/d" $(1); \
		echo "RTCONFIG_BCM5301X_TRAFFIC_MONITOR=y" >>$(1); \
	fi; \
	if [ "$(SPEEDTEST)" = "y" ]; then \
		sed -i "/RTCONFIG_SPEEDTEST/d" $(1); \
		echo "RTCONFIG_SPEEDTEST=y" >>$(1); \
	fi; \
	if [ "$(BCM_7114)" = "y" ]; then \
		sed -i "/RTCONFIG_BCM_7114/d" $(1); \
		echo "RTCONFIG_BCM_7114=y" >>$(1); \
		sed -i "/RTCONFIG_BCMBSD/d" $(1); \
		echo "RTCONFIG_BCMBSD=y" >>$(1); \
		sed -i "/RTCONFIG_WLEXE/d" $(1); \
		echo "RTCONFIG_WLEXE=y" >>$(1); \
	fi; \
	if [ "$(GMAC3)" = "y" ]; then \
		sed -i "/RTCONFIG_GMAC3/d" $(1); \
		echo "RTCONFIG_GMAC3=y" >>$(1); \
		sed -i "/RTCONFIG_BCMFA/d" $(1); \
		echo "# RTCONFIG_BCMFA is not set" >>$(1); \
	fi; \
	if [ "$(BCM9)" = "y" ]; then \
		sed -i "/RTCONFIG_BCM9/d" $(1); \
		echo "RTCONFIG_BCM9=y" >>$(1); \
		sed -i "/RTCONFIG_WLEXE/d" $(1); \
		echo "RTCONFIG_WLEXE=y" >>$(1); \
	fi; \
	if [ "$(BCM7)" = "y" ]; then \
		sed -i "/RTCONFIG_BCM7/d" $(1); \
		echo "RTCONFIG_BCM7=y" >>$(1); \
		sed -i "/RTCONFIG_TOAD/d" $(1); \
		echo "RTCONFIG_TOAD=y" >>$(1); \
		sed -i "/RTCONFIG_BCMBSD/d" $(1); \
		echo "RTCONFIG_BCMBSD=y" >>$(1); \
		sed -i "/RTCONFIG_GMAC3/d" $(1); \
		echo "# RTCONFIG_GMAC3 is not set" >>$(1); \
	fi; \
	if [ "$(HND_ROUTER)" = "y" ]; then \
		sed -i "/RTCONFIG_HND_ROUTER/d" $(1); \
		echo "RTCONFIG_HND_ROUTER=y" >>$(1); \
		sed -i "/RTCONFIG_EMF/d" $(1); \
		echo "RTCONFIG_EMF=y" >>$(1); \
		sed -i "/RTCONFIG_BCMBSD/d" $(1); \
		echo "RTCONFIG_BCMBSD=y" >>$(1); \
		sed -i "/RTCONFIG_LBR_AGGR/d" $(1); \
		echo "RTCONFIG_LBR_AGGR=y" >>$(1); \
		sed -i "/RTCONFIG_WLEXE/d" $(1); \
		echo "RTCONFIG_WLEXE=y" >>$(1); \
		sed -i "/RTCONFIG_HNDMFG/d" $(1); \
		if [ "$(HND_MFG)" = "y" ]; then \
			echo "RTCONFIG_HNDMFG=y" >>$(1); \
		else \
			echo "# RTCONFIG_HNDMFG is not set" >>$(1); \
		fi; \
		sed -i "/RTCONFIG_VISUALIZATION/d" $(1); \
		if [ "$(VISUALIZATION)" = "y" ]; then \
			echo "RTCONFIG_VISUALIZATION=y" >>$(1); \
		else \
			echo "# RTCONFIG_VISUALIZATION is not set" >>$(1); \
		fi; \
		if [ "$(DFS_US)" = "y" ]; then \
			echo "RTCONFIG_DFS_US=y" >>$(1); \
		else \
			echo "# RTCONFIG_DFS_US is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(HND_ROUTER_F1)" = "y" ]; then \
		sed -i "/RTCONFIG_WBD/d" $(1); \
		echo "RTCONFIG_WBD=y" >>$(1); \
	fi; \
	if [ "$(BUILD_BCM7)" = "y" ]; then \
		sed -i "/RTCONFIG_BUILDBCM7/d" $(1); \
		echo "RTCONFIG_BUILDBCM7=y" >>$(1); \
	fi; \
	if [ "$(DHDAP)" = "y" ]; then \
		sed -i "/RTCONFIG_DHDAP/d" $(1); \
		echo "RTCONFIG_DHDAP=y" >>$(1); \
	fi; \
	if [ "$(DPSTA)" = "y" ]; then \
		sed -i "/RTCONFIG_DPSTA/d" $(1); \
		echo "RTCONFIG_DPSTA=y" >>$(1); \
	fi; \
	if [ "$(ROMCFE)" = "y" ]; then \
		sed -i "/RTCONFIG_ROMCFE/d" $(1); \
		echo "RTCONFIG_ROMCFE=y" >>$(1); \
	fi; \
	if [ "$(ROMCCODE)" = "y" ]; then \
		sed -i "/RTCONFIG_ROMATECCODEFIX/d" $(1); \
		echo "RTCONFIG_ROMATECCODEFIX=y" >>$(1); \
	fi; \
	if [ "$(SSD)" = "y" ]; then \
		sed -i "/RTCONFIG_BCMSSD/d" $(1); \
		echo "RTCONFIG_BCMSSD=y" >>$(1); \
	fi; \
	if [ "$(HSPOT)" = "y" ]; then \
		sed -i "/RTCONFIG_HSPOT/d" $(1); \
		echo "RTCONFIG_HSPOT=y" >>$(1); \
	fi; \
	if [ "$(NVSIZE)" != "" ]; then \
		sed -i "/RTCONFIG_NV$(NVSIZE)/d" $(1); \
		echo "RTCONFIG_NV$(NVSIZE)=y" >>$(1); \
	fi; \
	if [ "$(RALINK)" = "y" ] || [ "$(QCA)" = "y" ]; then \
		if [ "$(NVRAM_SIZE)" != "" ]; then \
			sed -i "/RTCONFIG_NVRAM_SIZE/d" $(1); \
			echo "RTCONFIG_NVRAM_SIZE=`printf 0x%x $(NVRAM_SIZE)`" >>$(1); \
		fi; \
	fi; \
	if [ "$(BONDING)" = "y" ]; then \
		sed -i "/RTCONFIG_BONDING/d" $(1); \
		echo "RTCONFIG_BONDING=y" >>$(1); \
	fi; \
	if [ "$(WIFILOGO)" = "y" ]; then \
		sed -i "/RTCONFIG_WIFILOGO/d" $(1); \
		echo "RTCONFIG_WIFILOGO=y" >>$(1); \
	fi; \
	if [ "$(JFFS2USERICON)" = "y" ]; then \
		sed -i "/RTCONFIG_JFFS2USERICON/d" $(1); \
		echo "RTCONFIG_JFFS2USERICON=y" >>$(1); \
	fi; \
	if [ "$(SWITCH2)" = "RTL8365MB" ]; then \
		sed -i "/RTCONFIG_EXT_RTL8370MB/d" $(1); \
		echo "# RTCONFIG_EXT_RTL8370MB is not set" >>$(1); \
		sed -i "/RTCONFIG_EXT_RTL8365MB/d" $(1); \
		echo "RTCONFIG_EXT_RTL8365MB=y" >>$(1); \
	else \
	if [ "$(SWITCH2)" = "RTL8370MB" ]; then \
		sed -i "/RTCONFIG_EXT_RTL8365MB/d" $(1); \
		echo "# RTCONFIG_EXT_RTL8365MB is not set" >>$(1); \
		sed -i "/RTCONFIG_EXT_RTL8370MB/d" $(1); \
		echo "RTCONFIG_EXT_RTL8370MB=y" >>$(1); \
	else \
	if [ "$(SWITCH2)" = "BCM53134" ]; then \
		sed -i "/RTCONFIG_EXT_BCM53134/d" $(1); \
		echo "# RTCONFIG_EXT_BCM53134 is not set" >>$(1); \
		sed -i "/RTCONFIG_EXT_BCM53134/d" $(1); \
		echo "RTCONFIG_EXT_BCM53134=y" >>$(1); \
	else \
	if [ "$(SWITCH2)" = "" ]; then \
		sed -i "/RTCONFIG_EXT_RTL8365MB/d" $(1); \
		echo "# RTCONFIG_EXT_RTL8365MB is not set" >>$(1); \
		sed -i "/RTCONFIG_EXT_RTL8370MB/d" $(1); \
		echo "# RTCONFIG_EXT_RTL8370MB is not set" >>$(1); \
	fi; \
	fi; \
	fi; \
	fi; \
	if [ "$(TOR)" = "y" ]; then \
		sed -i "/RTCONFIG_TOR/d" $(1); \
		echo "RTCONFIG_TOR=y" >>$(1); \
	fi; \
	if [ "$(CFEZ)" = "y" ]; then \
		sed -i "/RTCONFIG_CFEZ/d" $(1); \
		echo "RTCONFIG_CFEZ=y" >>$(1); \
	fi; \
	if [ "$(TR069)" = "y" ]; then \
		sed -i "/RTCONFIG_TR069/d" $(1); \
		echo "RTCONFIG_TR069=y" >>$(1); \
	fi; \
	if [ "$(TR181)" = "y" ]; then \
		sed -i "/RTCONFIG_TR181/d" $(1); \
		echo "RTCONFIG_TR181=y" >>$(1); \
	fi; \
	if [ "$(STAINFO)" = "y" ]; then \
		sed -i "/RTCONFIG_STAINFO/d" $(1); \
		echo "RTCONFIG_STAINFO=y" >>$(1); \
	fi; \
	if [ "$(CLOUDCHECK)" = "y" ]; then \
		sed -i "/RTCONFIG_CLOUDCHECK/d" $(1); \
		echo "RTCONFIG_CLOUDCHECK=y" >>$(1); \
	fi; \
	if [ "$(GETREALIP)" = "y" ]; then \
		sed -i "/RTCONFIG_GETREALIP/d" $(1); \
		echo "RTCONFIG_GETREALIP=y" >>$(1); \
	fi; \
	if [ "$(BCM_MMC)" = "y" ]; then \
		sed -i "/RTCONFIG_MMC_LED/d" $(1); \
		echo "RTCONFIG_MMC_LED=y" >>$(1); \
	fi; \
	if [ "$(NATNL)" = "y" ]; then \
		sed -i "/RTCONFIG_TUNNEL/d" $(1); \
		echo "RTCONFIG_TUNNEL=y" >>$(1); \
 	fi; \
	if [ "$(NATNL_AICLOUD)" = "y" ]; then \
		sed -i "/RTCONFIG_TUNNEL/d" $(1); \
		echo "RTCONFIG_TUNNEL=y" >>$(1); \
		sed -i "/RTCONFIG_AICLOUD_TUNNEL/d" $(1); \
		echo "RTCONFIG_AICLOUD_TUNNEL=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_AICLOUD_TUNNEL/d" $(1); \
		echo "# RTCONFIG_AICLOUD_TUNNEL is not set" >>$(1); \
	fi; \
	if [ "$(NATNL_AIHOME)" = "y" ]; then \
		sed -i "/RTCONFIG_TUNNEL/d" $(1); \
		echo "RTCONFIG_TUNNEL=y" >>$(1); \
		sed -i "/RTCONFIG_AIHOME_TUNNEL/d" $(1); \
		echo "RTCONFIG_AIHOME_TUNNEL=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_AIHOME_TUNNEL/d" $(1); \
		echo "# RTCONFIG_AIHOME_TUNNEL is not set" >>$(1); \
	fi; \
	if [ "$(ERPTEST)" = "y" ]; then \
		sed -i "/RTCONFIG_ERP_TEST/d" $(1); \
		echo "RTCONFIG_ERP_TEST=y" >>$(1); \
	fi; \
	if [ "$(RESET_SWITCH)" = "y" ]; then \
		sed -i "/RTCONFIG_RESET_SWITCH/d" $(1); \
		echo "RTCONFIG_RESET_SWITCH=y" >>$(1); \
	fi; \
	if [ "$(DEF_AP)" = "y" ]; then \
		sed -i "/RTCONFIG_DEFAULT_AP_MODE/d" $(1); \
		echo "RTCONFIG_DEFAULT_AP_MODE=y" >>$(1); \
	fi; \
	if [ "$(DEF_REPEATER)" = "y" ]; then \
		sed -i "/RTCONFIG_DEFAULT_REPEATER_MODE/d" $(1); \
		echo "RTCONFIG_DEFAULT_REPEATER_MODE=y" >>$(1); \
	fi; \
	if [ "$(DHCP_OVERRIDE)" = "y" ]; then \
		sed -i "/RTCONFIG_DHCP_OVERRIDE/d" $(1); \
		echo "RTCONFIG_DHCP_OVERRIDE=y" >>$(1); \
	fi; \
	if [ "$(RES_GUI)" = "y" ]; then \
		sed -i "/RTCONFIG_RESTRICT_GUI/d" $(1); \
		echo "RTCONFIG_RESTRICT_GUI=y" >>$(1); \
	fi; \
	if [ "$(KEY_GUARD)" = "y" ]; then \
		sed -i "/RTCONFIG_KEY_GUARD/d" $(1); \
		echo "RTCONFIG_KEY_GUARD=y" >>$(1); \
	fi; \
	if [ "$(WTFAST)" = "y" ]; then \
		sed -i "/RTCONFIG_WTFAST/d" $(1); \
		echo "RTCONFIG_WTFAST=y" >>$(1); \
	fi; \
	if [ "$(IFTTT)" = "y" ]; then \
		sed -i "/RTCONFIG_IFTTT/d" $(1); \
		echo "RTCONFIG_IFTTT=y" >>$(1); \
		sed -i "/RTCONFIG_TUNNEL/d" $(1); \
		echo "RTCONFIG_TUNNEL=y" >>$(1); \
		sed -i "/RTCONFIG_AIHOME_TUNNEL/d" $(1); \
		echo "RTCONFIG_AIHOME_TUNNEL=y" >>$(1); \
	fi; \
	if [ "$(ALEXA)" = "y" ]; then \
		sed -i "/RTCONFIG_ALEXA/d" $(1); \
		echo "RTCONFIG_ALEXA=y" >>$(1); \
		sed -i "/RTCONFIG_TUNNEL/d" $(1); \
		echo "RTCONFIG_TUNNEL=y" >>$(1); \
		sed -i "/RTCONFIG_AIHOME_TUNNEL/d" $(1); \
		echo "RTCONFIG_AIHOME_TUNNEL=y" >>$(1); \
	fi; \
	if [ "$(REBOOT_SCHEDULE)" = "y" ]; then \
		sed -i "/RTCONFIG_REBOOT_SCHEDULE/d" $(1); \
		echo "RTCONFIG_REBOOT_SCHEDULE=y" >>$(1); \
	fi; \
	if [ "$(CAPTIVE_PORTAL)" = "y" ]; then \
		sed -i "/RTCONFIG_CAPTIVE_PORTAL/d" $(1); \
		echo "RTCONFIG_CAPTIVE_PORTAL=y" >>$(1); \
		sed -i "/RTCONFIG_COOVACHILLI/d" $(1); \
		echo "RTCONFIG_COOVACHILLI=y" >>$(1); \
		sed -i "/RTCONFIG_FREERADIUS/d" $(1); \
		echo "RTCONFIG_FREERADIUS=y" >>$(1); \
		if [ "$(CP_FREEWIFI)" = "y" ]; then \
			sed -i "/RTCONFIG_CP_FREEWIFI/d" $(1); \
			echo "RTCONFIG_CP_FREEWIFI=y" >>$(1); \
		else \
			sed -i "/RTCONFIG_CP_FREEWIFI/d" $(1); \
			echo "# RTCONFIG_CP_FREEWIFI is not set" >>$(1); \
		fi; \
		if [ "$(CP_ADVANCED)" = "y" ]; then \
			sed -i "/RTCONFIG_CP_ADVANCED/d" $(1); \
			echo "RTCONFIG_CP_ADVANCED=y" >>$(1); \
		else \
			sed -i "/RTCONFIG_CP_ADVANCED/d" $(1); \
			echo "# RTCONFIG_CP_ADVANCED is not set" >>$(1); \
		fi; \
	else \
		if [ "$(CHILLISPOT)" = "y" ]; then \
			sed -i "/RTCONFIG_CHILLISPOT/d" $(1); \
			echo "RTCONFIG_CHILLISPOT=y" >>$(1); \
		fi; \
		if [ "$(FREERADIUS)" = "y" ]; then \
			sed -i "/RTCONFIG_FREERADIUS/d" $(1); \
			echo "RTCONFIG_FREERADIUS=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(FBWIFI)" = "y" ]; then \
		sed -i "/RTCONFIG_FBWIFI/d" $(1); \
		echo "RTCONFIG_FBWIFI=y" >>$(1); \
	fi; \
	if [ "$(FORCE_AUTO_UPGRADE)" = "y" ]; then \
		sed -i "/RTCONFIG_FORCE_AUTO_UPGRADE/d" $(1); \
		echo "RTCONFIG_FORCE_AUTO_UPGRADE=y" >>$(1); \
	fi; \
	if [ "$(TUXERA_SMBD)" = "y" ]; then \
		sed -i "/RTCONFIG_TUXERA_SMBD/d" $(1); \
		echo "RTCONFIG_TUXERA_SMBD=y" >>$(1); \
	fi; \
	if [ "$(QUAGGA)" = "y" ]; then \
		sed -i "/RTCONFIG_QUAGGA/d" $(1); \
		echo "RTCONFIG_QUAGGA=y" >>$(1); \
	fi; \
	if [ "$(ASPMD)" = "y" ]; then \
		sed -i "/RTCONFIG_BCMASPMD/d" $(1); \
		echo "RTCONFIG_BCMASPMD=y" >>$(1); \
	fi; \
	if [ "$(BCMEVENTD)" = "y" ]; then \
		sed -i "/RTCONFIG_BCMEVENTD/d" $(1); \
		echo "RTCONFIG_BCMEVENTD=y" >>$(1); \
	fi; \
	if [ "$(BCM_MEVENT)" = "y" ]; then \
		sed -i "/RTCONFIG_BCM_MEVENT/d" $(1); \
		echo "RTCONFIG_BCM_MEVENT=y" >>$(1); \
	fi; \
	if [ "$(BCM_APPEVENTD)" = "y" ]; then \
		sed -i "/RTCONFIG_BCM_APPEVENTD/d" $(1); \
		echo "RTCONFIG_BCM_APPEVENTD=y" >>$(1); \
	fi; \
	if [ "$(WLCLMLOAD)" = "y" ]; then \
		sed -i "/RTCONFIG_WLCLMLOAD/d" $(1); \
		echo "RTCONFIG_WLCLMLOAD=y" >>$(1); \
	fi; \
	if [ "$(BCM_MUMIMO)" = "y" ] || [ "$(MTK_MUMIMO)" = "y" ]; then \
		sed -i "/RTCONFIG_MUMIMO/d" $(1); \
		echo "RTCONFIG_MUMIMO=y" >>$(1); \
	fi; \
	if [ "$(MUMIMO_5G)" = "y" ]; then \
		sed -i "/RTCONFIG_MUMIMO_5G/d" $(1); \
		echo "RTCONFIG_MUMIMO_5G=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_MUMIMO_5G/d" $(1); \
		echo "# RTCONFIG_MUMIMO_5G is not set" >>$(1); \
	fi; \
	if [ "$(MUMIMO_2G)" = "y" ]; then \
		sed -i "/RTCONFIG_MUMIMO_2G/d" $(1); \
		echo "RTCONFIG_MUMIMO_2G=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_MUMIMO_2G/d" $(1); \
		echo "# RTCONFIG_MUMIMO_2G is not set" >>$(1); \
	fi; \
	if [ "$(QAM256_2G)" = "y" ]; then \
		sed -i "/RTCONFIG_QAM256_2G/d" $(1); \
		echo "RTCONFIG_QAM256_2G=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_QAM256_2G/d" $(1); \
		echo "# RTCONFIG_QAM256_2G is not set" >>$(1); \
	fi; \
	if [ "$(MULTICASTIPTV)" = "y" ]; then \
		sed -i "/RTCONFIG_MULTICAST_IPTV/d" $(1); \
		echo "RTCONFIG_MULTICAST_IPTV=y" >>$(1); \
	fi; \
	if [ "$(VLAN)" = "y" ]; then \
		sed -i "/RTCONFIG_PORT_BASED_VLAN/d" $(1); \
		echo "RTCONFIG_PORT_BASED_VLAN=y" >>$(1); \
	fi; \
	if [ "$(VLAN_TAGGED_BASE)" = "y" ]; then \
		sed -i "/RTCONFIG_TAGGED_BASED_VLAN/d" $(1); \
		echo "RTCONFIG_TAGGED_BASED_VLAN=y" >>$(1); \
	fi; \
	if [ "$(MTK_NAND)" = "y" ]; then \
		sed -i "/RTCONFIG_MTK_NAND/d" $(1); \
		echo "RTCONFIG_MTK_NAND=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_MTK_NAND/d" $(1); \
		echo "# RTCONFIG_MTK_NAND is not set" >>$(1); \
	fi; \
	if [ "$(DISABLE_NETWORKMAP)" = "y" ]; then \
		sed -i "/RTCONFIG_DISABLE_NETWORKMAP/d" $(1); \
		echo "RTCONFIG_DISABLE_NETWORKMAP=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_DISABLE_NETWORKMAP/d" $(1); \
		echo "# RTCONFIG_DISABLE_NETWORKMAP is not set" >>$(1); \
	fi; \
	if [ "$(WAN_AT_P4)" = "y" ]; then \
		sed -i "/RTCONFIG_WAN_AT_P4/d" $(1); \
		echo "RTCONFIG_WAN_AT_P4=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_WAN_AT_P4/d" $(1); \
		echo "# RTCONFIG_WAN_AT_P4 is not set" >>$(1); \
	fi; \
	if [ "$(MTK_REP)" = "y" ]; then \
		sed -i "/RTCONFIG_MTK_REP/d" $(1); \
		echo "RTCONFIG_MTK_REP=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_MTK_REP/d" $(1); \
		echo "# RTCONFIG_MTK_REP is not set" >>$(1); \
	fi; \
	if [ "$(ATED122)" = "y" ]; then \
		sed -i "/RTCONFIG_ATED122/d" $(1); \
		echo "RTCONFIG_ATED122=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_ATED122/d" $(1); \
		echo "# RTCONFIG_ATED122 is not set" >>$(1); \
	fi; \
	if [ "$(EDCCA_NEW)" = "y" ]; then \
		sed -i "/RTCONFIG_RALINK_EDCCA/d" $(1); \
		echo "RTCONFIG_RALINK_EDCCA=y" >>$(1); \
	fi; \
	if [ "$(RT3883)" = "y" ]; then \
		sed -i "/RTCONFIG_RALINK_RT3883/d" $(1); \
		echo "RTCONFIG_RALINK_RT3883=y" >>$(1); \
	fi; \
	if [ "$(RT3052)" = "y" ]; then \
		sed -i "/RTCONFIG_RALINK_RT3052/d" $(1); \
		echo "RTCONFIG_RALINK_RT3052=y" >>$(1); \
	fi; \
	if [ "$(NOIPTV)" = "y" ]; then \
		sed -i "/RTCONFIG_NOIPTV/d" $(1); \
		echo "RTCONFIG_NOIPTV=y" >>$(1); \
	fi; \
	if [ "$(ATCOVER)" = "y" ]; then \
		sed -i "/RTCONFIG_AUTOCOVER_SIP/d" $(1); \
		echo "RTCONFIG_AUTOCOVER_SIP=y" >>$(1); \
	fi; \
	if [ "$(LAN50)" = "y" ]; then \
		sed -i "/RTCONFIG_DEFLAN50/d" $(1); \
		echo "RTCONFIG_DEFLAN50=y" >>$(1); \
	fi; \
	if [ "$(PERMISSION_MANAGEMENT)" = "y" ]; then \
		sed -i "/RTCONFIG_PERMISSION_MANAGEMENT/d" $(1); \
		echo "RTCONFIG_PERMISSION_MANAGEMENT=y" >>$(1); \
	fi; \
	if [ "$(DETWAN)" = "y" ]; then \
		sed -i "/RTCONFIG_DETWAN/d" $(1); \
		echo "RTCONFIG_DETWAN=y" >>$(1); \
	fi; \
	if [ "$(CFGSYNC)" = "y" ]; then \
		sed -i "/RTCONFIG_CFGSYNC/d" $(1); \
		echo "RTCONFIG_CFGSYNC=y" >>$(1); \
		if [ "$(MASTER_DET)" = "y" ]; then \
			sed -i "/RTCONFIG_MASTER_DET/d" $(1); \
			echo "RTCONFIG_MASTER_DET=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(LP5523)" = "y" ]; then \
		sed -i "/RTCONFIG_LP5523/d" $(1); \
		echo "RTCONFIG_LP5523=y" >>$(1); \
	fi; \
	if [ "$(RALINK)" = "y" -o "$(QCA)" = "y" -o "$(REALTEK)" = "y" ]; then \
		sed -i "/CONFIG_LIBBCM/d" $(1); \
		echo "# CONFIG_LIBBCM is not set" >>$(1); \
	fi; \
	if [ "$(NANO)" = "y" ]; then \
		sed -i "/RTCONFIG_NANO/d" $(1); \
		echo "RTCONFIG_NANO=y" >>$(1); \
	fi; \
	if [ "$(UPNPIGD2)" = "y" ]; then \
		sed -i "/RTCONFIG_IGD2/d" $(1); \
		echo "RTCONFIG_IGD2=y" >>$(1); \
	fi; \
	if [ "$(DNSSEC)" = "y" ]; then \
		sed -i "/RTCONFIG_DNSSEC/d" $(1); \
		echo "RTCONFIG_DNSSEC=y" >>$(1); \
	fi; \
	if [ "$(MERLINUPDATE)" = "y" ]; then\
		sed -i "/RTCONFIG_MERLINUPDATE/d" $(1); \
		echo "RTCONFIG_MERLINUPDATE=y" >>$(1); \
	fi; \
	if [ "$(DNSFILTER)" = "y" ]; then \
		sed -i "/RTCONFIG_DNSFILTER/d" $(1); \
		echo "RTCONFIG_DNSFILTER=y" >>$(1); \
	fi; \
	if [ "$(CFGSYNC)" = "y" ]; then \
		sed -i "/RTCONFIG_CFGSYNC/d" $(1); \
		echo "RTCONFIG_CFGSYNC=y" >>$(1); \
	fi; \
	if [ "$(WEBMON)" = "y" ]; then \
		sed -i "/RTCONFIG_WEBMON/d" $(1); \
		echo "RTCONFIG_WEBMON=y" >>$(1); \
	fi; \
	if [ "$(BACKUP_LOG)" = "y" ]; then \
		sed -i "/RTCONFIG_BACKUP_LOG/d" $(1); \
		echo "RTCONFIG_BACKUP_LOG=y" >>$(1); \
		sed -i "/RTCONFIG_NOTIFICATION_CENTER/d" $(1); \
		echo "RTCONFIG_NOTIFICATION_CENTER=y" >>$(1); \
	fi; \
	if [ "$(LETSENCRYPT)" = "y" ]; then \
		sed -i "/RTCONFIG_LETSENCRYPT/d" $(1); \
		echo "RTCONFIG_LETSENCRYPT=y" >>$(1); \
	fi; \
	if [ "$(WLCSCAN_RSSI)" = "y" ]; then \
		sed -i "/RTCONFIG_WLCSCAN_RSSI/d" $(1); \
		echo "RTCONFIG_WLCSCAN_RSSI=y" >>$(1); \
	else\
		echo "# RTCONFIG_WLCSCAN_RSSI is not set" >>$(1); \
	fi; \
	if [ "$(BT_CONN)" = "y" ]; then \
		sed -i "/RTCONFIG_BT_CONN/d" $(1); \
		echo "RTCONFIG_BT_CONN=y" >>$(1); \
	fi; \
	if [ "$(SINGLE_SSID)" = "y" ]; then \
		sed -i "/RTCONFIG_SINGLE_SSID/d" $(1); \
		echo "RTCONFIG_SINGLE_SSID=y" >>$(1); \
	fi; \
	if [ "$(SSID_AMAPS)" = "y" ]; then \
		sed -i "/RTCONFIG_SSID_AMAPS/d" $(1); \
		echo "RTCONFIG_SSID_AMAPS=y" >>$(1); \
	fi; \
	if [ "$(QCA)" = "y" ] && [ "$(MESH)" = "y" ]; then \
		sed -i "/RTCONFIG_WIFI_SON/d" $(1); \
		echo "RTCONFIG_WIFI_SON=y" >>$(1); \
		if [ "$(ETHBACKHAUL)" = "y" ]; then \
			sed -i "/RTCONFIG_ETHBACKHAUL/d" $(1); \
			echo "RTCONFIG_ETHBACKHAUL=y" >>$(1); \
		fi; \
		if [ "$(DUAL_BACKHAUL)" = "y" ]; then \
			sed -i "/RTCONFIG_DUAL_BACKHAUL/d" $(1); \
			echo "RTCONFIG_DUAL_BACKHAUL=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(AUTHSUPP)" = "y" ]; then \
		sed -i "/RTCONFIG_AUTHSUPP/d" $(1); \
		echo "RTCONFIG_AUTHSUPP=y" >>$(1); \
	fi; \
	if [ "$(VPN_FUSION)" = "y" ]; then \
		sed -i "/RTCONFIG_VPN_FUSION/d" $(1); \
		echo "RTCONFIG_VPN_FUSION=y" >>$(1); \
	fi; \
	if [ "$(MTK_8021X3000)" = "y" ]; then \
		sed -i "/RTCONFIG_MTK_8021X3000/d" $(1); \
		echo "RTCONFIG_MTK_8021X3000=y" >>$(1); \
	fi; \
	if [ "$(DBG_BLUECAVE_OBD)" = "y" ]; then \
		sed -i "/RTCONFIG_DBG_BLUECAVE_OBD/d" $(1); \
		echo "RTCONFIG_DBG_BLUECAVE_OBD=y" >>$(1); \
	fi; \
	if [ "$(AMAS)" = "y" ]; then \
		sed -i "/RTCONFIG_AMAS/d" $(1); \
		echo "RTCONFIG_AMAS=y" >>$(1); \
		if [ "$(LANTIQ)" != "y" ]; then \
			sed -i "/RTCONFIG_DPSTA/d" $(1); \
			echo "RTCONFIG_DPSTA=y" >>$(1); \
		fi; \
		if [ "$(DISABLE_REPEATER_UI)" = "y" ] ; then \
			sed -i "/RTCONFIG_DISABLE_REPEATER_UI/d" $(1); \
			echo "# RTCONFIG_DISABLE_REPEATER_UI is not set" >>$(1); \
		fi; \
		sed -i "/RTCONFIG_CFGSYNC/d" $(1); \
		echo "RTCONFIG_CFGSYNC=y" >>$(1); \
		sed -i "/RTCONFIG_MASTER_DET/d" $(1); \
		echo "RTCONFIG_MASTER_DET=y" >>$(1); \
		sed -i "/RTCONFIG_ADV_RAST/d" $(1); \
		echo "RTCONFIG_ADV_RAST=y" >>$(1); \
		sed -i "/RTCONFIG_WLCEVENTD/d" $(1); \
		echo "RTCONFIG_WLCEVENTD=y" >>$(1); \
		sed -i "/RTCONFIG_LIBASUSLOG/d" $(1); \
		echo "RTCONFIG_LIBASUSLOG=y" >>$(1); \
	fi; \
	if [ "$(NO_SELECT_CHANNEL)" = "y" ]; then \
		sed -i "/RTCONFIG_NO_SELECT_CHANNEL/d" $(1); \
		echo "RTCONFIG_NO_SELECT_CHANNEL=y" >>$(1); \
	else \
		sed -i "/RTCONFIG_NO_SELECT_CHANNEL/d" $(1); \
		echo "# RTCONFIG_NO_SELECT_CHANNEL is not set" >>$(1); \
	fi; \
	if [ "$(USB_SWAP)" = "y" ]; then \
		sed -i "/RTCONFIG_USB_SWAP/d" $(1); \
		echo "RTCONFIG_USB_SWAP=y" >>$(1); \
	fi; \
	if [ "$(SW_DEVLED)" = "y" ]; then \
		sed -i "/RTCONFIG_SW_DEVLED/d" $(1); \
		echo "RTCONFIG_SW_DEVLED=y" >>$(1); \
	fi; \
	if [ "$(LYRA_HIDE)" = "y" ]; then \
		sed -i "/RTCONFIG_LYRA_HIDE/d" $(1); \
		echo "RTCONFIG_LYRA_HIDE=y" >>$(1); \
	fi; \
	if [ "$(NVRAM_ENCRYPT)" = "y" ]; then \
		sed -i "/RTCONFIG_NVRAM_ENCRYPT/d" $(1); \
		echo "RTCONFIG_NVRAM_ENCRYPT=y" >>$(1); \
	fi; \
	if [ "$(WIFI_PROXY)" = "y" ]; then \
		sed -i "/RTCONFIG_WIFI_PROXY/d" $(1); \
		echo "RTCONFIG_WIFI_PROXY=y" >>$(1); \
	fi; \
	if [ "$(HD_SPINDOWN)" = "y" ]; then \
		sed -i "/RTCONFIG_HD_SPINDOWN/d" $(1); \
		echo "RTCONFIG_HD_SPINDOWN=y" >>$(1); \
	fi; \
	if [ "$(ADTBW)" = "y" ]; then \
		sed -i "/RTCONFIG_ADTBW/d" $(1); \
		echo "RTCONFIG_ADTBW=y" >>$(1); \
	fi; \
	if [ "$(TXBF_BAND3ONLY)" = "y" ]; then \
		sed -i "/RTCONFIG_TXBF_BAND3ONLY/d" $(1); \
		echo "RTCONFIG_TXBF_BAND3ONLY=y" >>$(1); \
	fi; \
	if [ "$(SW_HW_AUTH)" = "y" ]; then \
		sed -i "/RTCONFIG_SW_HW_AUTH\>/d" $(1); \
		echo "RTCONFIG_SW_HW_AUTH=y" >>$(1); \
	fi; \
	if [ "$(LIBASUSLOG)" = "y" ]; then \
		sed -i "/RTCONFIG_LIBASUSLOG\>/d" $(1); \
		echo "RTCONFIG_LIBASUSLOG=y" >>$(1); \
	fi; \
	)
	$(call platformRouterOptions, $(1))
endef

define BusyboxOptions
	@( \
	if [ "$(CONFIG_LINUX26)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_2_4_MODULES/d" $(1); \
		echo "# CONFIG_FEATURE_2_4_MODULES is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_LSMOD_PRETTY_2_6_OUTPUT/d" $(1); \
		echo "CONFIG_FEATURE_LSMOD_PRETTY_2_6_OUTPUT=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
		echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
		sed -i "/CONFIG_MKNOD/d" $(1); \
		echo "CONFIG_MKNOD=y" >>$(1); \
	fi; \
	if [ "$(NO_CIFS)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_MOUNT_CIFS/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_CIFS is not set" >>$(1); \
	fi; \
	if [ "$(BBEXTRAS)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_SORT_BIG/d" $(1); \
		echo "CONFIG_FEATURE_SORT_BIG=y" >>$(1); \
		sed -i "/CONFIG_CLEAR/d" $(1); \
		echo "CONFIG_CLEAR=y" >>$(1); \
		sed -i "/CONFIG_SETCONSOLE/d" $(1); \
		echo "CONFIG_SETCONSOLE=y" >>$(1); \
		if [ "$(CONFIG_LINUX26)" = "y" ]; then \
			sed -i "/CONFIG_FEATURE_SYSLOGD_READ_BUFFER_SIZE/d" $(1); \
			echo "CONFIG_FEATURE_SYSLOGD_READ_BUFFER_SIZE=512" >>$(1); \
		fi; \
		if [ "$(DSL)" = "y" ]; then \
			sed -i "/CONFIG_TFTP/d" $(1); \
			echo "CONFIG_TFTP=y" >>$(1); \
			sed -i "/CONFIG_TFTPD/d" $(1); \
			echo "# CONFIG_TFTPD is not set" >>$(1); \
			sed -i "/CONFIG_FEATURE_TFTP_GET/d" $(1); \
			echo "CONFIG_FEATURE_TFTP_GET=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_TFTP_PUT/d" $(1); \
			echo "CONFIG_FEATURE_TFTP_PUT=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_TFTP_BLOCKSIZE/d" $(1); \
			echo "# CONFIG_FEATURE_TFTP_BLOCKSIZE is not set" >>$(1); \
			sed -i "/CONFIG_FEATURE_TFTP_PROGRESS_BAR/d" $(1); \
			echo "# CONFIG_FEATURE_TFTP_PROGRESS_BAR is not set" >>$(1); \
			sed -i "/CONFIG_TFTP_DEBUG/d" $(1); \
			echo "# CONFIG_TFTP_DEBUG is not set" >>$(1); \
			if [ "$(DSL_TCLINUX)" = "y" ]; then \
				sed -i "/CONFIG_TELNET/d" $(1); \
				echo "CONFIG_TELNET=y" >>$(1); \
				sed -i "/CONFIG_FEATURE_TELNET_TTYPE/d" $(1); \
				echo "# CONFIG_FEATURE_TELNET_TTYPE is not set" >>$(1); \
				sed -i "/CONFIG_FEATURE_TELNET_AUTOLOGIN/d" $(1); \
				echo "# CONFIG_FEATURE_TELNET_AUTOLOGIN is not set" >>$(1); \
				sed -i "/CONFIG_TELNETD/d" $(1); \
				echo "CONFIG_TELNETD=y" >>$(1); \
				sed -i "/CONFIG_FEATURE_TELNETD_STANDALONE/d" $(1); \
				echo "CONFIG_FEATURE_TELNETD_STANDALONE=y" >>$(1); \
				sed -i "/CONFIG_FEATURE_TELNETD_INETD_WAIT/d" $(1); \
				echo "# CONFIG_FEATURE_TELNETD_INETD_WAIT is not set" >>$(1); \
				sed -i "/CONFIG_FTPGET/d" $(1); \
				echo "CONFIG_FTPGET=y" >>$(1); \
				sed -i "/CONFIG_FTPPUT/d" $(1); \
				echo "CONFIG_FTPPUT=y" >>$(1); \
				sed -i "/CONFIG_FEATURE_FTPGETPUT_LONG_OPTIONS/d" $(1); \
				echo "CONFIG_FEATURE_FTPGETPUT_LONG_OPTIONS=y" >>$(1); \
				sed -i "/CONFIG_SPLIT/d" $(1); \
				echo "CONFIG_SPLIT=y" >>$(1); \
			fi;\
		fi; \
	fi; \
	if [ "$(USB)" = "USB" ]; then \
		if [ "$(NFS)" = "y" ]; then \
			sed -i "/CONFIG_FEATURE_MOUNT_NFS/d" $(1); \
			echo "CONFIG_FEATURE_MOUNT_NFS=y" >>$(1); \
		fi; \
		if [ "$(DISK_MONITOR)" = "y" ]; then \
			sed -i "/CONFIG_FSCK/d" $(1); \
			echo "CONFIG_FSCK=y" >>$(1); \
			if [ "$(E2FSPROGS)" != "y" ]; then \
				sed -i "/CONFIG_E2FSCK/d" $(1); \
				echo "CONFIG_E2FSCK=y" >>$(1); \
			fi; \
		fi; \
		if [ "$(USBEXTRAS)" = "y" ]; then \
			sed -i "/CONFIG_FSCK_MINIX/d" $(1); \
			echo "CONFIG_FSCK_MINIX=y" >>$(1); \
			sed -i "/CONFIG_MKSWAP/d" $(1); \
			echo "CONFIG_MKSWAP=y" >>$(1); \
			sed -i "/CONFIG_FLOCK/d" $(1); \
			echo "CONFIG_FLOCK=y" >>$(1); \
			sed -i "/CONFIG_FSYNC/d" $(1); \
			echo "CONFIG_FSYNC=y" >>$(1); \
			sed -i "/CONFIG_UNZIP/d" $(1); \
			echo "CONFIG_UNZIP=y" >>$(1); \
			if [ "$(CONFIG_LINUX26)" = "y" ]; then \
				sed -i "/CONFIG_LSUSB/d" $(1); \
				echo "CONFIG_LSUSB=y" >>$(1); \
				sed -i "/CONFIG_FEATURE_WGET_STATUSBAR/d" $(1); \
				echo "CONFIG_FEATURE_WGET_STATUSBAR=y" >>$(1); \
				sed -i "/CONFIG_FEATURE_VERBOSE_USAGE/d" $(1); \
				echo "CONFIG_FEATURE_VERBOSE_USAGE=y" >>$(1); \
			fi; \
		fi; \
		if [ "$(NO_MKTOOLS)" != "y" ]; then \
			if [ "$(E2FSPROGS)" != "y" ]; then \
				sed -i "/CONFIG_MKE2FS/d" $(1); \
				echo "CONFIG_MKE2FS=y" >>$(1); \
			fi; \
			sed -i "/CONFIG_FDISK/d" $(1); \
			echo "CONFIG_FDISK=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_FDISK_WRITABLE/d" $(1); \
			echo "CONFIG_FEATURE_FDISK_WRITABLE=y" >>$(1); \
		fi; \
		if [ "$(GOBI)" = "y" ]; then \
			sed -i "/CONFIG_TFTP /d" $(1); \
			echo "CONFIG_TFTP=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_TFTP_GET/d" $(1); \
			echo "CONFIG_FEATURE_TFTP_GET=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_TFTP_PUT/d" $(1); \
			echo "CONFIG_FEATURE_TFTP_PUT=y" >>$(1); \
			sed -i "/CONFIG_FEATURE_TFTP_BLOCKSIZE/d" $(1); \
			echo "CONFIG_FEATURE_TFTP_BLOCKSIZE=y" >>$(1); \
		fi; \
	else \
		sed -i "/CONFIG_FEATURE_MOUNT_LOOP/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_LOOP is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
		echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_MOUNT_LABEL/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_LABEL is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_MOUNT_FSTAB/d" $(1); \
		echo "# CONFIG_FEATURE_MOUNT_FSTAB is not set" >>$(1); \
		sed -i "/CONFIG_VOLUMEID/d" $(1); \
		echo "# CONFIG_VOLUMEID is not set" >>$(1); \
		sed -i "/CONFIG_BLKID/d" $(1); \
		echo "# CONFIG_BLKID is not set" >>$(1); \
		sed -i "/CONFIG_SWAPONOFF/d" $(1); \
		echo "# CONFIG_SWAPONOFF is not set" >>$(1); \
		sed -i "/CONFIG_TRUE/d" $(1); \
		echo "# CONFIG_TRUE is not set" >>$(1); \
	fi; \
	if [ "$(IPV6SUPP)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_IPV6/d" $(1); \
		echo "CONFIG_FEATURE_IPV6=y" >>$(1); \
		sed -i "/CONFIG_PING6/d" $(1); \
		echo "CONFIG_PING6=y" >>$(1); \
		sed -i "/CONFIG_TRACEROUTE6/d" $(1); \
		echo "CONFIG_TRACEROUTE6=y" >>$(1); \
	fi; \
	if [ "$(SNMPD)" = "y" ]; then \
		sed -i "/CONFIG_TFTP/d" $(1); \
		echo "CONFIG_TFTP=y" >>$(1); \
		sed -i "/CONFIG_TFTPD/d" $(1); \
		echo "# CONFIG_TFTPD is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_TFTP_GET/d" $(1); \
		echo "CONFIG_FEATURE_TFTP_GET=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TFTP_PUT/d" $(1); \
		echo "CONFIG_FEATURE_TFTP_PUT=y" >>$(1); \
		sed -i "/CONFIG_TFTP_DEBUG/d" $(1); \
		echo "# CONFIG_TFTP_DEBUG is not set" >>$(1); \
	fi; \
	if [ "$(RTN11P)" = "y" ] || [ "$(RTN300)" = "y" ]; then \
		sed -i "/CONFIG_LESS/d" $(1); \
		echo "# CONFIG_LESS is not set" >>$(1); \
		sed -i "/CONFIG_DU\b/d" $(1); \
		echo "# CONFIG_DU is not set" >>$(1); \
		sed -i "/CONFIG_HEAD/d" $(1); \
		echo "# CONFIG_HEAD is not set" >>$(1); \
		sed -i "/CONFIG_TAIL/d" $(1); \
		echo "# CONFIG_TAIL is not set" >>$(1); \
		sed -i "/CONFIG_BASENAME/d" $(1); \
		echo "# CONFIG_BASENAME is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
		echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
		sed -i "/CONFIG_BLKID/d" $(1); \
		echo "# CONFIG_BLKID is not set" >>$(1); \
		sed -i "/CONFIG_TELNET\b/d" $(1); \
		echo "# CONFIG_TELNET is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_LS_COLOR\b/d" $(1); \
		echo "# CONFIG_FEATURE_LS_COLOR is not set" >>$(1); \
		sed -i "/CONFIG_CUT/d" $(1); \
		echo "# CONFIG_CUT is not set" >>$(1); \
		sed -i "/CONFIG_CROND/d" $(1); \
		echo "# CONFIG_CROND is not set" >>$(1); \
		sed -i "/CONFIG_MD5SUM/d" $(1); \
		echo "# CONFIG_MD5SUM is not set" >>$(1); \
		sed -i "/CONFIG_AWK/d" $(1); \
		echo "# CONFIG_AWK is not set" >>$(1); \
		sed -i "/CONFIG_WC/d" $(1); \
		echo "# CONFIG_WC is not set" >>$(1); \
	fi; \
	if [ "$(IPQ40XX)" = "y" ]; then \
		sed -i "/CONFIG_DEVMEM/d" $(1); \
		echo "CONFIG_DEVMEM=y" >>$(1); \
	fi; \
	if [ "$(MAPAC1300)" = "y" ] || [ "$(MAPAC2200)" = "y" ] || [ "$(VZWAC1300)" = "y" ]; then \
		sed -i "/CONFIG_TFTP/d" $(1); \
		echo "CONFIG_TFTP=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TFTP_GET/d" $(1); \
		echo "CONFIG_FEATURE_TFTP_GET=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TFTP_PUT/d" $(1); \
		echo "CONFIG_FEATURE_TFTP_PUT=y" >>$(1); \
		sed -i "/CONFIG_TFTPD/d" $(1); \
		echo "# CONFIG_TFTPD is not set" >>$(1); \
		sed -i "/CONFIG_TFTP_DEBUG/d" $(1); \
		echo "# CONFIG_TFTP_DEBUG is not set" >>$(1); \
		sed -i "/CONFIG_TELNET\b/d" $(1); \
		echo "CONFIG_TELNET=y" >>$(1); \
	fi; \
	if [ "$(SLIM)" = "y" ]; then \
		sed -i "/CONFIG_AWK/d" $(1); \
		echo "# CONFIG_AWK is not set" >>$(1); \
		sed -i "/CONFIG_BASENAME/d" $(1); \
		echo "# CONFIG_BASENAME is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
		echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
		sed -i "/CONFIG_BLKID/d" $(1); \
		echo "# CONFIG_BLKID is not set" >>$(1); \
		sed -i "/CONFIG_TELNET\b/d" $(1); \
		echo "# CONFIG_TELNET is not set" >>$(1); \
		sed -i "/CONFIG_ARPING/d" $(1); \
		echo "# CONFIG_ARPING is not set" >>$(1); \
		sed -i "/CONFIG_FEATURE_LS_COLOR/d" $(1); \
		echo "# CONFIG_FEATURE_LS_COLOR is not set" >>$(1); \
	else \
		if [ "$(SFP)" = "y" ]; then \
			sed -i "/CONFIG_LESS/d" $(1); \
			echo "# CONFIG_LESS is not set" >>$(1); \
			sed -i "/CONFIG_GZIP/d" $(1); \
			echo "# CONFIG_GZIP is not set" >>$(1); \
			sed -i "/CONFIG_DU\b/d" $(1); \
			echo "# CONFIG_DU is not set" >>$(1); \
			sed -i "/CONFIG_TAIL/d" $(1); \
			echo "# CONFIG_TAIL is not set" >>$(1); \
			sed -i "/CONFIG_BASENAME/d" $(1); \
			echo "# CONFIG_BASENAME is not set" >>$(1); \
			sed -i "/CONFIG_FEATURE_DEVFS/d" $(1); \
			echo "# CONFIG_FEATURE_DEVFS is not set" >>$(1); \
			sed -i "/CONFIG_BLKID/d" $(1); \
			echo "# CONFIG_BLKID is not set" >>$(1); \
			sed -i "/CONFIG_TELNET\b/d" $(1); \
			echo "# CONFIG_TELNET is not set" >>$(1); \
			sed -i "/CONFIG_ARPING/d" $(1); \
			echo "# CONFIG_ARPING is not set" >>$(1); \
			sed -i "/CONFIG_FEATURE_LS_COLOR\b/d" $(1); \
			echo "# CONFIG_FEATURE_LS_COLOR is not set" >>$(1); \
			if [ "$(MODEM)" != "y" ]; then \
				sed -i "/CONFIG_HEAD/d" $(1); \
				echo "# CONFIG_HEAD is not set" >>$(1); \
			fi; \
			if [ "$(SFP4M)" = "y" ]; then \
				sed -i "/CONFIG_TAR/d" $(1); \
				echo "# CONFIG_TAR is not set" >>$(1); \
				sed -i "/CONFIG_DD/d" $(1); \
				echo "# CONFIG_DD is not set" >>$(1); \
				sed -i "/CONFIG_SORT/d" $(1); \
				echo "# CONFIG_SORT is not set" >>$(1); \
				sed -i "/CONFIG_DMESG/d" $(1); \
				echo "# CONFIG_DMESG is not set" >>$(1); \
				sed -i "/CONFIG_CROND/d" $(1); \
				echo "# CONFIG_CROND is not set" >>$(1); \
				sed -i "/CONFIG_EXPR_MATH_SUPPORT_64/d" $(1); \
				echo "# CONFIG_EXPR_MATH_SUPPORT_64 is not set" >>$(1); \
				sed -i "/CONFIG_MD5SUM/d" $(1); \
				echo "# CONFIG_MD5SUM is not set" >>$(1); \
				sed -i "/CONFIG_TAIL/d" $(1); \
				echo "# CONFIG_TAIL is not set" >>$(1); \
				sed -i "/CONFIG_VI/d" $(1); \
				echo "# CONFIG_VI is not set" >>$(1); \
				if [ "$(MODEM)" != "y" ]; then \
					sed -i "/CONFIG_AWK/d" $(1); \
					echo "# CONFIG_AWK is not set" >>$(1); \
					sed -i "/CONFIG_FIND/d" $(1); \
					echo "# CONFIG_FIND is not set" >>$(1); \
					echo "# CONFIG_FINDFS is not set" >>$(1); \
					sed -i "/CONFIG_CUT/d" $(1); \
					echo "# CONFIG_CUT is not set" >>$(1); \
					sed -i "/CONFIG_WC/d" $(1); \
					echo "# CONFIG_WC is not set" >>$(1); \
				fi; \
			fi; \
		else \
			sed -i "/CONFIG_FEATURE_LS_COLOR\b/d" $(1); \
			echo "CONFIG_FEATURE_LS_COLOR=y" >>$(1); \
			sed -i "/CONFIG_SENDMAIL/d" $(1); \
			echo "CONFIG_SENDMAIL=y" >>$(1); \
			if [ "$(HND_MFG)" = "y" ]; then \
				sed -i "/CONFIG_FEATURE_LS_COLOR_IS_DEFAULT/d" $(1); \
				echo "# CONFIG_FEATURE_LS_COLOR_IS_DEFAULT is not set" >>$(1); \
			fi; \
		fi; \
	fi; \
	if [ "$(DISKTEST)" = "y" ]; then \
		sed -i "/CONFIG_HDPARM/d" $(1); \
		echo "CONFIG_HDPARM=y" >>$(1); \
	fi; \
	if [ "$(BCMSMP)" = "y" ] || [ "$(ALPINE)" = "y" ] || [ "$(LANTIQ)" = "y" ] ; then \
		sed -i "/CONFIG_FEATURE_TOP_SMP_CPU/d" $(1); \
		echo "CONFIG_FEATURE_TOP_SMP_CPU=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TOP_DECIMALS/d" $(1); \
		echo "CONFIG_FEATURE_TOP_DECIMALS=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TOP_SMP_PROCESS/d" $(1); \
		echo "CONFIG_FEATURE_TOP_SMP_PROCESS=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TOPMEM/d" $(1); \
		echo "CONFIG_FEATURE_TOPMEM=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_SHOW_THREADS/d" $(1); \
		echo "CONFIG_FEATURE_SHOW_THREADS=y" >>$(1); \
	fi; \
	if [ "$(ALPINE)" = "y" ] ; then \
		sed -i "/CONFIG_STTY/d" $(1); \
		echo "CONFIG_STTY=y" >>$(1); \
	fi; \
	if [ "$(LANTIQ)" = "y" ] ; then \
		sed -i "/CONFIG_LSPCI/d" $(1); \
		echo "CONFIG_LSPCI=y" >>$(1); \
		sed -i "/CONFIG_LSUSB/d" $(1); \
		echo "CONFIG_LSUSB=y" >>$(1); \
	fi; \
	if [ "$(LANTIQ)" = "y" ] ; then \
		sed -i "/CONFIG_XARGS/d" $(1); \
		echo "CONFIG_XARGS=y" >>$(1); \
	fi; \
	if [ "$(LANTIQ)" = "y" ] ; then \
		sed -i "/CONFIG_TFTP/d" $(1); \
		echo "CONFIG_TFTP=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TFTP_GET/d" $(1); \
		echo "CONFIG_FEATURE_TFTP_GET=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_TFTP_PUT/d" $(1); \
		echo "CONFIG_FEATURE_TFTP_PUT=y" >>$(1); \
		sed -i "/CONFIG_TFTPD/d" $(1); \
		echo "# CONFIG_TFTPD is not set" >>$(1); \
		sed -i "/CONFIG_TFTP_DEBUG/d" $(1); \
		echo "# CONFIG_TFTP_DEBUG is not set" >>$(1); \
	fi; \
	if [ "$(WANRED_LED)" = "y" ]; then \
		sed -i "/CONFIG_ARPING/d" $(1); \
		echo "CONFIG_ARPING=y" >>$(1); \
	fi; \
	if [ "$(HTTPS)" = "y" ]; then \
		sed -i "/CONFIG_WGET/d" $(1); \
		echo "# CONFIG_WGET is not set" >>$(1); \
	fi; \
	if [ "$(HND_ROUTER)" = "y" ]; then \
		sed -i "/CONFIG_FEATURE_BASH_IS_ASH/d" $(1); \
		echo "CONFIG_FEATURE_BASH_IS_ASH=y" >>$(1); \
		sed -i "/CONFIG_FEATURE_BASH_IS_NONE/d" $(1); \
		echo "# CONFIG_FEATURE_BASH_IS_NONE is not set" >>$(1); \
	fi; \
	)
	$(call platformBusyboxOptions, $(1))
endef

define extraKernelConfig
	@( \
	if [ ! -z "$(EXTRA_KERNEL_YES_CONFIGS)" ] ; then \
		for c in $(EXTRA_KERNEL_YES_CONFIGS) ; do \
			sed -i "/CONFIG_$${c}/d" $(1); \
			echo "CONFIG_$${c}=y" >>$(1); \
		done \
	fi; \
	if [ ! -z "$(EXTRA_KERNEL_NO_CONFIGS)" ] ; then \
		for c in $(EXTRA_KERNEL_NO_CONFIGS) ; do \
			sed -i "/CONFIG_$${c}/d" $(1); \
			echo "# CONFIG_$${c} is not set" >>$(1); \
		done \
	fi; \
	if [ ! -z "$(EXTRA_KERNEL_MOD_CONFIGS)" ] ; then \
		for c in $(EXTRA_KERNEL_MOD_CONFIGS) ; do \
			sed -i "/CONFIG_$${c}/d" $(1); \
			echo "CONFIG_$${c}=m" >>$(1); \
		done \
	fi; \
	if [ ! -z "$(EXTRA_KERNEL_VAL_CONFIGS)" ] ; then \
		for c in $(EXTRA_KERNEL_VAL_CONFIGS) ; do \
			sed -i "/CONFIG_$${c}/d" $(1); \
			echo "CONFIG_$${c}" >>$(1); \
		done \
	fi; \
	)
endef

define KernelConfig
	@( \
	sed -i "/CONFIG_PPP_DEFLATE/d" $(1); \
	echo "CONFIG_PPP_DEFLATE=m" >>$(1); \
	sed -i "/CONFIG_PPP_FILTER/d" $(1); \
	echo "# CONFIG_PPP_FILTER is not set" >>$(1); \
	sed -i "/CONFIG_PPP_MULTILINK/d" $(1); \
	echo "# CONFIG_PPP_MULTILINK is not set" >>$(1); \
if [ "$(TUNEK)" != "n" ]; then \
	if [ "$(RALINK)" = "y" ] || [ "$(QCA)" = "y" ]; then \
		sed -i "/CONFIG_NVRAM_SIZE/d" $(1); \
		echo "CONFIG_NVRAM_SIZE=`printf 0x%x $(NVRAM_SIZE)`" >>$(1); \
	fi; \
	sed -i "/CONFIG_CC_OPTIMIZE_FOR_SIZE/d" $(1); \
	if [ "$(KERN_SIZE_OPT)" = "y" ]; then \
		echo "CONFIG_CC_OPTIMIZE_FOR_SIZE=y" >>$(1); \
	else \
		echo "# CONFIG_CC_OPTIMIZE_FOR_SIZE is not set" >>$(1); \
	fi; \
	if [ "$(CONFIG_LINUX26)" = "y" ] && [ "$(MIPS32)" = "r2" ]; then \
		sed -i "/CONFIG_CPU_MIPS32_R1/d" $(1); \
		echo "# CONFIG_CPU_MIPS32_R1 is not set" >>$(1); \
		sed -i "/CONFIG_CPU_MIPS32_R2/d" $(1); \
		echo "CONFIG_CPU_MIPS32_R2=y" >>$(1); \
		sed -i "/CONFIG_CPU_MIPSR1/d" $(1); \
		echo "CONFIG_CPU_MIPSR2=y" >>$(1); \
	fi; \
	if [ "$(RTN11P)" = "y" ] || [ "$(RTN300)" = "y" ]; then \
		sed -i "/CONFIG_USB/d" $(1); \
		echo "# CONFIG_USB is not set" >>$(1); \
		sed -i "/CONFIG_USB_SUPPORT/d" $(1); \
		echo "# CONFIG_USB_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_USB_ARCH_HAS_OHCI/d" $(1); \
		echo "# CONFIG_USB_ARCH_HAS_OHCI is not set" >>$(1); \
		sed -i "/CONFIG_USB_ARCH_HAS_EHCI/d" $(1); \
		echo "# CONFIG_USB_ARCH_HAS_EHCI is not set" >>$(1); \
		sed -i "/CONFIG_SWAP/d" $(1); \
		echo "# CONFIG_SWAP is not set" >>$(1); \
		sed -i "/CONFIG_RD_GZIP/d" $(1); \
		echo "# CONFIG_RD_GZIP is not set" >>$(1); \
		sed -i "/CONFIG_SCSI/d" $(1); \
		echo "# CONFIG_SCSI is not set" >>$(1); \
		sed -i "/CONFIG_EXT2_FS/d" $(1); \
		echo "# CONFIG_EXT2_FS is not set" >>$(1); \
		sed -i "/CONFIG_EXT3_FS/d" $(1); \
		echo "# CONFIG_EXT3_FS is not set" >>$(1); \
		sed -i "/CONFIG_FAT_FS/d" $(1); \
		echo "# CONFIG_FAT_FS is not set" >>$(1); \
		sed -i "/CONFIG_VFAT_FS/d" $(1); \
		echo "# CONFIG_VFAT_FS is not set" >>$(1); \
		sed -i "/CONFIG_REISERFS_FS/d" $(1); \
		echo "# CONFIG_REISERFS_FS is not set" >>$(1); \
		sed -i "/CONFIG_JFFS2_FS/d" $(1); \
		echo "# CONFIG_JFFS2_FS is not set" >>$(1); \
		sed -i "/CONFIG_FUSE_FS/d" $(1); \
		echo "# CONFIG_FUSE_FS is not set" >>$(1); \
		sed -i "/CONFIG_CONFIGFS_FS/d" $(1); \
		echo "# CONFIG_CONFIGFS_FS is not set" >>$(1); \
		sed -i "/CONFIG_SERIAL_NONSTANDARD/d" $(1); \
		echo "# CONFIG_SERIAL_NONSTANDARD is not set" >>$(1); \
		sed -i "/CONFIG_NETWORK_FILESYSTEMS/d" $(1); \
		echo "# CONFIG_NETWORK_FILESYSTEMS is not set" >>$(1); \
		sed -i "/CONFIG_CC_OPTIMIZE_FOR_SIZE/d" $(1); \
		echo "CONFIG_CC_OPTIMIZE_FOR_SIZE=y" >>$(1); \
		sed -i "/CONFIG_KALLSYMS/d" $(1); \
		echo "# CONFIG_KALLSYMS is not set" >>$(1); \
		sed -i "/CONFIG_RALINK_TIMER/d" $(1); \
		echo "# CONFIG_RALINK_TIMER is not set" >>$(1); \
		sed -i "/CONFIG_BUG/d" $(1); \
		echo "# CONFIG_BUG is not set" >>$(1); \
	fi; \
	if [ "$(PLN12)" = "y" ] || [ "$(PLAC56)" = "y" ] || [ "$(RPAC66)" = "y" ] || [ "$(RPAC51)" = "y" ] ; then \
		sed -i "/CONFIG_USB/d" $(1); \
		echo "# CONFIG_USB is not set" >>$(1); \
		sed -i "/CONFIG_USB_SUPPORT/d" $(1); \
		echo "# CONFIG_USB_SUPPORT is not set" >>$(1); \
		sed -i "/CONFIG_USB_ARCH_HAS_OHCI/d" $(1); \
		echo "# CONFIG_USB_ARCH_HAS_OHCI is not set" >>$(1); \
		sed -i "/CONFIG_USB_ARCH_HAS_EHCI/d" $(1); \
		echo "# CONFIG_USB_ARCH_HAS_EHCI is not set" >>$(1); \
		sed -i "/CONFIG_SWAP/d" $(1); \
		echo "# CONFIG_SWAP is not set" >>$(1); \
		sed -i "/CONFIG_RD_GZIP/d" $(1); \
		echo "# CONFIG_RD_GZIP is not set" >>$(1); \
		sed -i "/CONFIG_SCSI/d" $(1); \
		echo "# CONFIG_SCSI is not set" >>$(1); \
		sed -i "/CONFIG_EXT2_FS/d" $(1); \
		echo "# CONFIG_EXT2_FS is not set" >>$(1); \
		sed -i "/CONFIG_EXT3_FS/d" $(1); \
		echo "# CONFIG_EXT3_FS is not set" >>$(1); \
		sed -i "/CONFIG_FAT_FS/d" $(1); \
		echo "# CONFIG_FAT_FS is not set" >>$(1); \
		sed -i "/CONFIG_VFAT_FS/d" $(1); \
		echo "# CONFIG_VFAT_FS is not set" >>$(1); \
		sed -i "/CONFIG_REISERFS_FS/d" $(1); \
		echo "# CONFIG_REISERFS_FS is not set" >>$(1); \
		sed -i "/CONFIG_JFFS2_FS/d" $(1); \
		echo "# CONFIG_JFFS2_FS is not set" >>$(1); \
		sed -i "/CONFIG_FUSE_FS/d" $(1); \
		echo "# CONFIG_FUSE_FS is not set" >>$(1); \
		sed -i "/CONFIG_CONFIGFS_FS/d" $(1); \
		echo "# CONFIG_CONFIGFS_FS is not set" >>$(1); \
		sed -i "/CONFIG_SERIAL_NONSTANDARD/d" $(1); \
		echo "# CONFIG_SERIAL_NONSTANDARD is not set" >>$(1); \
		sed -i "/CONFIG_NETWORK_FILESYSTEMS/d" $(1); \
		echo "# CONFIG_NETWORK_FILESYSTEMS is not set" >>$(1); \
		sed -i "/CONFIG_CC_OPTIMIZE_FOR_SIZE/d" $(1); \
		echo "CONFIG_CC_OPTIMIZE_FOR_SIZE=y" >>$(1); \
		sed -i "/CONFIG_KALLSYMS/d" $(1); \
		echo "# CONFIG_KALLSYMS is not set" >>$(1); \
	fi; \
	if [ "$(AP_CARRIER_DETECTION)" = "y" ]; then \
		sed -i "/CONFIG_RALINK_TIMER_DFS/d" $(1); \
		echo "CONFIG_RALINK_TIMER_DFS=y" >>$(1); \
		sed -i "/CONFIG_RT2860V2_AP_DFS/d" $(1); \
		echo "CONFIG_RT2860V2_AP_DFS=y" >>$(1); \
		sed -i "/CONFIG_RT2860V2_AP_CARRIER/d" $(1); \
		echo "CONFIG_RT2860V2_AP_CARRIER=y" >>$(1); \
		sed -i "/CONFIG_RTPCI_AP_CARRIER/d" $(1); \
		echo "CONFIG_RTPCI_AP_CARRIER=y" >>$(1); \
	else \
		sed -i "/CONFIG_RALINK_TIMER_DFS/d" $(1); \
		echo "# CONFIG_RALINK_TIMER_DFS is not set" >>$(1); \
		sed -i "/CONFIG_RT2860V2_AP_DFS/d" $(1); \
		echo "# CONFIG_RT2860V2_AP_DFS is not set" >>$(1); \
		sed -i "/CONFIG_RT2860V2_AP_CARRIER/d" $(1); \
		echo "# CONFIG_RT2860V2_AP_CARRIER is not set" >>$(1); \
		sed -i "/CONFIG_RTPCI_AP_CARRIER/d" $(1); \
		echo "# CONFIG_RTPCI_AP_CARRIER is not set" >>$(1); \
	fi; \
	if [ "$(CONFIG_LINUX30)" = "y" ]; then \
		sed -i "/CONFIG_USB_XHCI_HCD/d" $(1); \
		echo "# CONFIG_USB_XHCI_HCD is not set" >>$(1); \
		if [ "$(USB)" = "USB" ]; then \
			if [ "$(XHCI)" = "y" ]; then \
				if [ "$(ALPINE)" = "y" ] || [ "$(LANTIQ)" = "y" ] ; then \
					sed -i "/CONFIG_USB_XHCI_HCD/d" $(1); \
					echo "CONFIG_USB_XHCI_HCD=m" >>$(1); \
					sed -i "/CONFIG_USB_XHCI_HCD_DEBUGGING/d" $(1); \
					echo "# CONFIG_USB_XHCI_HCD_DEBUGGING is not set" >>$(1); \
					sed -i "/CONFIG_USB_EHCI_HCD/d" $(1); \
					echo "CONFIG_USB_EHCI_HCD=m" >>$(1); \
					sed -i "/CONFIG_USB_EHCI_HCD_PLATFORM/d" $(1); \
					echo "CONFIG_USB_EHCI_HCD_PLATFORM=y">>$(1); \
					sed -i "/CONFIG_USB_EHCI_ROOT_HUB_TT/d" $(1); \
					echo "CONFIG_USB_EHCI_ROOT_HUB_TT=y" >>$(1); \
					sed -i "/CONFIG_USB_EHCI_TT_NEWSCHED/d" $(1); \
					echo "CONFIG_USB_EHCI_TT_NEWSCHED=y" >>$(1); \
				else \
					sed -i "/CONFIG_USB_XHCI_HCD/d" $(1); \
					echo "CONFIG_USB_XHCI_HCD=y" >>$(1); \
					sed -i "/CONFIG_USB_XHCI_HCD_DEBUGGING/d" $(1); \
					echo "# CONFIG_USB_XHCI_HCD_DEBUGGING is not set" >>$(1); \
				fi; \
			fi; \
		fi; \
	fi; \
	if [ "$(IPV6SUPP)" = "y" ]; then \
		sed -i "/CONFIG_IPV6 is not set/d" $(1); \
		echo "CONFIG_IPV6=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_IPTABLES/d" $(1); \
		echo "CONFIG_IP6_NF_IPTABLES=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_MATCH_RT/d" $(1); \
		echo "CONFIG_IP6_NF_MATCH_RT=y" >>$(1); \
		sed -i "/CONFIG_IP6_NF_FILTER/d" $(1); \
		echo "CONFIG_IP6_NF_FILTER=m" >>$(1); \
		sed -i "/CONFIG_IP6_NF_TARGET_LOG/d" $(1); \
		echo "CONFIG_IP6_NF_TARGET_LOG=m" >>$(1); \
		if [ "$(CONFIG_BCMWL5)" = "y" ] && [ "$(ARM)" != "y" ]; then \
			sed -i "/CONFIG_IP6_NF_TARGET_SKIPLOG/d" $(1); \
			echo "CONFIG_IP6_NF_TARGET_SKIPLOG=m" >>$(1); \
		fi; \
		sed -i "/CONFIG_IP6_NF_TARGET_REJECT\>/d" $(1); \
		echo "CONFIG_IP6_NF_TARGET_REJECT=m" >>$(1); \
		sed -i "/CONFIG_IP6_NF_MANGLE/d" $(1); \
		echo "CONFIG_IP6_NF_MANGLE=m" >>$(1); \
		if [ "$(CONFIG_LINUX26)" = "y" ]; then \
			sed -i "/CONFIG_NF_CONNTRACK_IPV6/d" $(1); \
			echo "CONFIG_NF_CONNTRACK_IPV6=y" >>$(1); \
			sed -i "/CONFIG_IPV6_ROUTER_PREF/d" $(1); \
			echo "CONFIG_IPV6_ROUTER_PREF=y" >>$(1); \
			sed -i "/CONFIG_IPV6_SIT\b/d" $(1); \
			echo "CONFIG_IPV6_SIT=m" >>$(1); \
			sed -i "/CONFIG_IPV6_SIT_6RD/d" $(1); \
			echo "CONFIG_IPV6_SIT_6RD=y" >>$(1); \
			sed -i "/CONFIG_IPV6_MULTIPLE_TABLES/d" $(1); \
			echo "CONFIG_IPV6_MULTIPLE_TABLES=y" >>$(1); \
			sed -i "/CONFIG_IP6_NF_TARGET_ROUTE/d" $(1); \
			echo "CONFIG_IP6_NF_TARGET_ROUTE=m" >>$(1); \
			sed -i "/CONFIG_IPV6_MROUTE\b/d" $(1); \
			echo "CONFIG_IPV6_MROUTE=y" >>$(1); \
		fi; \
		if [ "$(CONFIG_LINUX30)" = "y" ]; then \
			sed -i "/CONFIG_IP6_NF_CONNTRACK/d" $(1); \
			echo "CONFIG_IP6_NF_CONNTRACK=m" >>$(1); \
			sed -i "/CONFIG_IPV6_ROUTER_PREF/d" $(1); \
			echo "CONFIG_IPV6_ROUTER_PREF=y" >>$(1); \
			sed -i "/CONFIG_IPV6_SIT\b/d" $(1); \
			echo "CONFIG_IPV6_SIT=m" >>$(1); \
			sed -i "/CONFIG_IPV6_SIT_6RD/d" $(1); \
			echo "CONFIG_IPV6_SIT_6RD=y" >>$(1); \
			sed -i "/CONFIG_IPV6_MULTIPLE_TABLES/d" $(1); \
			echo "CONFIG_IPV6_MULTIPLE_TABLES=y" >>$(1); \
			sed -i "/CONFIG_IP6_NF_FTP/d" $(1); \
			echo "CONFIG_IP6_NF_FTP=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_MATCH_LIMIT/d" $(1); \
			echo "CONFIG_IP6_NF_MATCH_LIMIT=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_MATCH_CONDITION/d" $(1); \
			echo "CONFIG_IP6_NF_MATCH_CONDITION=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_MATCH_MAC/d" $(1); \
			echo "CONFIG_IP6_NF_MATCH_MAC=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_MATCH_MULTIPORT/d" $(1); \
			echo "CONFIG_IP6_NF_MATCH_MULTIPORT=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_MATCH_MARK/d" $(1); \
			echo "CONFIG_IP6_NF_MATCH_MARK=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_MATCH_LENGTH/d" $(1); \
			echo "CONFIG_IP6_NF_MATCH_LENGTH=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_MATCH_STATE/d" $(1); \
			echo "CONFIG_IP6_NF_MATCH_STATE=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_TARGET_MARK/d" $(1); \
			echo "CONFIG_IP6_NF_TARGET_MARK=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_TARGET_TCPMSS/d" $(1); \
			echo "CONFIG_IP6_NF_TARGET_TCPMSS=m" >>$(1); \
			sed -i "/CONFIG_IP6_NF_TARGET_ROUTE/d" $(1); \
			echo "CONFIG_IP6_NF_TARGET_ROUTE=m" >>$(1); \
			sed -i "/CONFIG_IPV6_MROUTE\b/d" $(1); \
			echo "CONFIG_IPV6_MROUTE=y" >>$(1); \
		fi; \
	else \
		sed -i "/CONFIG_IPV6/d" $(1); \
		echo "# CONFIG_IPV6 is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_BCM57XX/d" $(1); \
	if [ "$(BCM57)" = "y" ]; then \
		sed -i "/CONFIG_ET_ALL_PASSIVE/d" $(1); \
		echo "CONFIG_BCM57XX=m" >>$(1); \
		echo "# CONFIG_ET_ALL_PASSIVE_ON is not set" >>$(1); \
		echo "CONFIG_ET_ALL_PASSIVE_RUNTIME=y" >>$(1); \
	else \
		echo "# CONFIG_BCM57XX is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_WL_USE_HIGH/d" $(1); \
	sed -i "/CONFIG_WL_USBAP/d" $(1); \
	if [ "$(USBAP)" = "y" ]; then \
		echo "CONFIG_WL_USE_HIGH=y" >> $(1); \
		echo "CONFIG_WL_USBAP=y" >>$(1); \
	else \
		echo "# CONFIG_WL_USE_HIGH is not set" >> $(1); \
		echo "# CONFIG_WL_USBAP is not set" >>$(1); \
	fi; \
	if [ "$(CONFIG_LINUX26)" = "y" ] && [ "$(EBTABLES)" = "y" ]; then \
		sed -i "/CONFIG_BRIDGE_NF_EBTABLES/d" $(1); \
		echo "CONFIG_BRIDGE_NF_EBTABLES=m" >>$(1); \
		if [ "$(IPV6SUPP)" = "y" ]; then \
			sed -i "/CONFIG_BRIDGE_EBT_IP6/d" $(1); \
			echo "CONFIG_BRIDGE_EBT_IP6=m" >>$(1); \
		fi; \
	fi; \
	sed -i "/CONFIG_NVRAM_64K/d" $(1); \
	if [ "$(NVRAM_64K)" = "y" ]; then \
		echo "CONFIG_NVRAM_64K=y" >>$(1); \
	else \
		echo "# CONFIG_NVRAM_64K is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_LOCALE2012/d" $(1); \
	if [ "$(LOCALE2012)" = "y" ]; then \
		echo "CONFIG_LOCALE2012=y" >>$(1); \
	else \
		echo "# CONFIG_LOCALE2012 is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_N56U_SR2/d" $(1); \
	if [ "$(N56U_SR2)" = "y" ]; then \
		echo "CONFIG_N56U_SR2=y" >>$(1); \
	else \
		echo "# CONFIG_N56U_SR2 is not set" >>$(1); \
	fi; \
	if [ "$(EXT4FS)" = "y" ]; then \
		sed -i "/CONFIG_EXT4_FS/d" $(1); \
		echo "CONFIG_EXT4_FS=m" >>$(1); \
		sed -i "/CONFIG_EXT4_FS_XATTR/d" $(1); \
		echo "CONFIG_EXT4_FS_XATTR=y" >>$(1); \
		sed -i "/CONFIG_EXT4_FS_POSIX_ACL/d" $(1); \
		echo "# CONFIG_EXT4_FS_POSIX_ACL is not set" >>$(1); \
		sed -i "/CONFIG_EXT4_FS_SECURITY/d" $(1); \
		echo "# CONFIG_EXT4_FS_SECURITY is not set" >>$(1); \
		sed -i "/CONFIG_EXT4_DEBUG/d" $(1); \
		echo "# CONFIG_EXT4_DEBUG is not set" >>$(1); \
	else \
		sed -i "/CONFIG_EXT4_FS/d" $(1); \
		echo "# CONFIG_EXT4_FS is not set" >>$(1); \
	fi; \
fi; \
	if [ "$(SHP)" = "y" ] || [ "$(LFP)" = "y" ]; then \
		if [ "$(HND_ROUTER)" != "y" ]; then \
			sed -i "/CONFIG_IP_NF_LFP/d" $(1); \
			echo "CONFIG_IP_NF_LFP=y" >>$(1); \
		fi; \
 	fi; \
	if [ "$(DNSMQ)" = "y" ]; then \
		sed -i "/CONFIG_IP_NF_DNSMQ/d" $(1); \
		echo "CONFIG_IP_NF_DNSMQ=y" >>$(1); \
	fi; \
	if [ "$(USB)" = "" ]; then \
		sed -i "/CONFIG_MSDOS_PARTITION/d" $(1); \
		echo "# CONFIG_MSDOS_PARTITION is not set" >>$(1); \
		sed -i "/CONFIG_EFI_PARTITION/d" $(1); \
		echo "# CONFIG_EFI_PARTITION is not set" >>$(1); \
	else \
		if [ "$(PRINTER)" != "y" ]; then \
			sed -i "/CONFIG_USB_PRINTER/d" $(1); \
			echo "# CONFIG_USB_PRINTER is not set" >>$(1); \
		fi; \
		if [ "$(MODEM)" = "y" ]; then \
			if [ "$(BECEEM)" = "y" ]; then \
				sed -i "/CONFIG_USB_BECEEM/d" $(1); \
				echo "CONFIG_USB_BECEEM=m" >>$(1); \
			fi; \
			if [ "$(LESSMODEM)" = "y" ]; then \
				sed -i "/CONFIG_HSO/d" $(1); \
				echo "# CONFIG_HSO is not set" >>$(1); \
				sed -i "/CONFIG_USB_IPHETH/d" $(1); \
				echo "# CONFIG_USB_IPHETH is not set" >>$(1); \
			fi; \
			if [ "$(GOBI)" = "y" ] && [ "$(MULTIMODEM)" != "y" ]; then \
				sed -i "/CONFIG_USB_SERIAL/d" $(1); \
				echo "# CONFIG_USB_SERIAL is not set" >>$(1); \
				sed -i "/CONFIG_USB_NET_AX8817X/d" $(1); \
				echo "# CONFIG_USB_NET_AX8817X is not set" >>$(1); \
				sed -i "/CONFIG_USB_NET_CDCETHER/d" $(1); \
				echo "CONFIG_USB_NET_CDCETHER=m" >>$(1); \
				sed -i "/CONFIG_USB_NET_CDC_NCM/d" $(1); \
				echo "# CONFIG_USB_NET_CDC_NCM is not set" >>$(1); \
				sed -i "/CONFIG_USB_NET_CDC_MBIM/d" $(1); \
				echo "# CONFIG_USB_NET_CDC_MBIM is not set" >>$(1); \
				sed -i "/CONFIG_USB_NET_RNDIS_HOST/d" $(1); \
				echo "# CONFIG_USB_NET_RNDIS_HOST is not set" >>$(1); \
				sed -i "/CONFIG_USB_NET_QMI_WWAN/d" $(1); \
				echo "# CONFIG_USB_NET_QMI_WWAN is not set" >>$(1); \
				sed -i "/CONFIG_USB_IPHETH/d" $(1); \
				echo "# CONFIG_USB_IPHETH is not set" >>$(1); \
				sed -i "/CONFIG_USB_WDM/d" $(1); \
				echo "# CONFIG_USB_WDM is not set" >>$(1); \
			fi; \
		else \
			sed -i "/CONFIG_USB_SERIAL/d" $(1); \
			echo "# CONFIG_USB_SERIAL is not set" >>$(1); \
			sed -i "/CONFIG_USB_ACM/d" $(1); \
			echo "# CONFIG_USB_ACM is not set" >>$(1); \
			sed -i "/CONFIG_USB_USBNET/d" $(1); \
			echo "# CONFIG_USB_USBNET is not set" >>$(1); \
			sed -i "/CONFIG_USB_IPHETH/d" $(1); \
			echo "# CONFIG_USB_IPHETH is not set" >>$(1); \
			sed -i "/CONFIG_USB_WDM/d" $(1); \
			echo "# CONFIG_USB_WDM is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(ARMCPUSMP)" = "up" ]; then \
		sed -i "/CONFIG_GENERIC_CLOCKEVENTS_BROADCAST/d" $(1); \
		echo "CONFIG_HAVE_LATENCYTOP_SUPPORT=y" >>$(1); \
		sed -i "/CONFIG_GENERIC_LOCKBREAK/d" $(1); \
		echo "CONFIG_BROKEN_ON_SMP=y" >>$(1); \
		sed -i "/CONFIG_TREE_RCU/d" $(1); \
		echo "# CONFIG_TREE_RCU is not set" >>$(1); \
		sed -i "/CONFIG_TREE_PREEMPT_RCU/d" $(1); \
		echo "CONFIG_TREE_PREEMPT_RCU=y" >>$(1); \
		sed -i "/CONFIG_TINY_RCU/d" $(1); \
		echo "# CONFIG_TINY_RCU is not set" >>$(1); \
		sed -i "/CONFIG_USE_GENERIC_SMP_HELPERS/d" $(1); \
		sed -i "/CONFIG_STOP_MACHINE/d" $(1); \
		sed -i "/CONFIG_MUTEX_SPIN_ON_OWNER/d" $(1); \
		echo "# CONFIG_MUTEX_SPIN_ON_OWNER is not set" >>$(1); \
		sed -i "/CONFIG_ARM_ERRATA_742230/d" $(1); \
		sed -i "/CONFIG_ARM_ERRATA_742231/d" $(1); \
		sed -i "/CONFIG_ARM_ERRATA_720789/d" $(1); \
		sed -i "/CONFIG_SMP\b/d" $(1); \
		echo "# CONFIG_SMP is not set" >>$(1); \
		sed -i "/CONFIG_NR_CPUS=2/d" $(1); \
		sed -i "/CONFIG_HOTPLUG_CPU/d" $(1); \
		sed -i "/CONFIG_RPS=y/d" $(1); \
	fi; \
	if [ "$(ALPINE)" = "y" ]; then \
		sed -i "/CONFIG_SENSORS_AMC6821/d" $(1); \
		echo "CONFIG_SENSORS_AMC6821=m" >>$(1); \
		sed -i "/CONFIG_THERMAL_HWMON/d" $(1); \
		echo "CONFIG_THERMAL_HWMON=y" >>$(1); \
	fi ; \
	if [ "$(DUALWAN)" = "y" ]; then \
		if [ "$(CONFIG_LINUX26)" = "y" ] || [ "$(CONFIG_LINUX30)" = "y" ]; then \
			sed -i "/CONFIG_IP_ADVANCED_ROUTER/d" $(1); \
			echo "CONFIG_IP_ADVANCED_ROUTER=y" >>$(1); \
			if [ "$(ALPINE)" = "y" ] || [ "$(LANTIQ)" = "y" ] ; then \
				echo "# CONFIG_IP_FIB_TRIE_STATS is not set" >>$(1); \
			fi ; \
			sed -i "/CONFIG_ASK_IP_FIB_HASH/d" $(1); \
			echo "CONFIG_ASK_IP_FIB_HASH=y" >>$(1); \
			sed -i "/CONFIG_IP_FIB_TRIE\b/d" $(1); \
			echo "# CONFIG_IP_FIB_TRIE is not set" >>$(1); \
			sed -i "/CONFIG_IP_MULTIPLE_TABLES/d" $(1); \
			echo "CONFIG_IP_MULTIPLE_TABLES=y" >>$(1); \
			sed -i "/CONFIG_IP_ROUTE_MULTIPATH\>/d" $(1); \
			echo "CONFIG_IP_ROUTE_MULTIPATH=y" >>$(1); \
			sed -i "/CONFIG_IP_ROUTE_MULTIPATH_CACHED/d" $(1); \
			echo "# CONFIG_IP_ROUTE_MULTIPATH_CACHED is not set" >>$(1); \
			sed -i "/CONFIG_IP_ROUTE_VERBOSE/d" $(1); \
			echo "# CONFIG_IP_ROUTE_VERBOSE is not set" >>$(1); \
			sed -i "/CONFIG_IP_MROUTE_MULTIPLE_TABLES/d" $(1); \
			echo "CONFIG_IP_MROUTE_MULTIPLE_TABLES=y" >>$(1); \
			sed -i "/CONFIG_NETFILTER_XT_MATCH_STATISTIC/d" $(1); \
			echo "CONFIG_NETFILTER_XT_MATCH_STATISTIC=y" >>$(1); \
		fi ; \
	fi; \
	if [ "$(BT_CONN)" = "y" ]; then \
		if [ "$(LANTIQ)" = "y" ]; then \
			sed -i "/CONFIG_BT\b/d" $(1); \
			echo "CONFIG_BT=m" >>$(1); \
			sed -i "/CONFIG_BT_RFCOMM/d" $(1); \
			echo "# CONFIG_BT_RFCOMM is not set" >>$(1); \
			sed -i "/CONFIG_BT_BNEP/d" $(1); \
			echo "# CONFIG_BT_BNEP is not set" >>$(1); \
			sed -i "/CONFIG_BT_HIDP/d" $(1); \
			echo "# CONFIG_BT_HIDP is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIBTUSB/d" $(1); \
			echo "CONFIG_BT_HCIBTUSB=m" >>$(1); \
			sed -i "/CONFIG_BT_HCIUART/d" $(1); \
			echo "# CONFIG_BT_HCIUART is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIBCM203X/d" $(1); \
			echo "# CONFIG_BT_HCIBCM203X is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIBPA10X/d" $(1); \
			echo "# CONFIG_BT_HCIBPA10X is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIBFUSB/d" $(1); \
			echo "# CONFIG_BT_HCIBFUSB is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIVHCI/d" $(1); \
			echo "# CONFIG_BT_HCIVHCI is not set" >>$(1); \
			sed -i "/CONFIG_BT_MRVL/d" $(1); \
			echo "# CONFIG_BT_MRVL is not set" >>$(1); \
			sed -i "/CONFIG_BT_ATH3K/d" $(1); \
			echo "CONFIG_BT_ATH3K=m" >>$(1); \
		elif [ "$(ALPINE)" = "yyy" ]; then \
			sed -i "/CONFIG_BT\b/d" $(1); \
			echo "CONFIG_BT=y" >>$(1); \
			sed -i "/CONFIG_BT_RFCOMM\b/d" $(1); \
			echo "# CONFIG_BT_RFCOMM is not set" >>$(1); \
			sed -i "/CONFIG_BT_BNEP\b/d" $(1); \
			echo "# CONFIG_BT_BNEP is not set" >>$(1); \
			sed -i "/CONFIG_BT_HIDP\b/d" $(1); \
			echo "# CONFIG_BT_HIDP is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIBTUSB\b/d" $(1); \
			echo "# CONFIG_BT_HCIBTUSB is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIUART\b/d" $(1); \
			echo "CONFIG_BT_HCIUART=y" >>$(1); \
			sed -i "/CONFIG_BT_HCIUART_H4\b/d" $(1); \
			echo "CONFIG_BT_HCIUART_H4=y" >>$(1); \
			sed -i "/CONFIG_BT_HCIUART_BCSP\b/d" $(1); \
			echo "# CONFIG_BT_HCIUART_BCSP is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIUART_RTKH5\b/d" $(1); \
			echo "CONFIG_BT_HCIUART_RTKH5=y" >>$(1); \
			sed -i "/CONFIG_BT_HCIUART_ATH3K\b/d" $(1); \
			echo "# CONFIG_BT_HCIUART_ATH3K is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIUART_LL\b/d" $(1); \
			echo "# CONFIG_BT_HCIUART_LL is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIUART_3WIRE\b/d" $(1); \
			echo "# CONFIG_BT_HCIUART_3WIRE is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIBCM203X\b/d" $(1); \
			echo "# CONFIG_BT_HCIBCM203X is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIBPA10X\b/d" $(1); \
			echo "# CONFIG_BT_HCIBPA10X is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIBFUSB\b/d" $(1); \
			echo "# CONFIG_BT_HCIBFUSB is not set" >>$(1); \
			sed -i "/CONFIG_BT_HCIVHCI\b/d" $(1); \
			echo "# CONFIG_BT_HCIVHCI is not set" >>$(1); \
			sed -i "/CONFIG_BT_MRVL\b/d" $(1); \
			echo "# CONFIG_BT_MRVL is not set" >>$(1); \
		fi ; \
	fi ; \
	sed -i "/CONFIG_CFE_NVRAM_CHK/d" $(1); \
	if [ "$(CFE_NVRAM_CHK)" = "y" ]; then \
		echo "CONFIG_CFE_NVRAM_CHK=y" >>$(1); \
	else \
		echo "# CONFIG_CFE_NVRAM_CHK is not set" >>$(1); \
	fi; \
	if [ "$(DEBUG)" = "y" ] || [ "$(GDB)" = "y" ]; then \
		sed -i "/CONFIG_ELF_CORE/d" $(1); \
		echo "CONFIG_ELF_CORE=y" >>$(1); \
		if [ "$(BCMWL6A)" = "y" ]; then \
			sed -i "/CONFIG_CORE_DUMP_DEFAULT_ELF_HEADERS/d" $(1); \
			echo "# CONFIG_CORE_DUMP_DEFAULT_ELF_HEADERS is not set" >>$(1); \
		fi; \
	fi; \
	sed -i "/CONFIG_DUAL_TRX/d" $(1); \
	if [ "$(DUAL_TRX)" = "y" ]; then \
		echo "CONFIG_DUAL_TRX=y" >>$(1); \
	else \
		echo "# CONFIG_DUAL_TRX is not set" >>$(1); \
	fi; \
	if [ "$(DUMP_OOPS_MSG)" = "y" ]; then \
		if [ "$(BUILD_NAME)" = "RT-AC66U" ] || [ "$(BUILD_NAME)" = "RT-N66U" ]; then \
			echo "CONFIG_DUMP_PREV_OOPS_MSG=y" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_ADDR=0x07FFE000" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_LEN=0x2000" >>$(1); \
		elif [ "$(BUILD_NAME)" = "RT-N65U" ]; then \
			echo "CONFIG_DUMP_PREV_OOPS_MSG=y" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_ADDR=0x01810000" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_LEN=0x2000" >>$(1); \
		elif [ "$(BCM_7114)" = "y" ] || [ "$(BCM7)" = "y" ]; then \
			echo "CONFIG_DUMP_PREV_OOPS_MSG=y" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_ADDR=0x80000000" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_LEN=0x3000" >>$(1); \
		elif [ "$(BUILD_NAME)" = "RT-AC56S" ]; then \
			echo "CONFIG_DUMP_PREV_OOPS_MSG=y" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_ADDR=0xC0522000" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_LEN=0x3000" >>$(1); \
		elif [ "$(BCM9)" = "y" ]; then \
			echo "CONFIG_DUMP_PREV_OOPS_MSG=y" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_ADDR=0x87000000" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_LEN=0x2000" >>$(1); \
		elif [ "$(ARM)" = "y" ]; then \
			echo "CONFIG_DUMP_PREV_OOPS_MSG=y" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_ADDR=0xC0000000" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_LEN=0x2000" >>$(1); \
		elif [ "$(RALINK)" = "y" ]; then \
			echo "CONFIG_DUMP_PREV_OOPS_MSG=y" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_ADDR=0x03300000" >>$(1); \
			echo "CONFIG_DUMP_PREV_OOPS_MSG_BUF_LEN=0x2000" >>$(1); \
		elif [ "$(QCA)" = "y" ]; then \
			echo "move to platform.mak" > /dev/null ; \
		else \
			echo "# CONFIG_DUMP_PREV_OOPS_MSG is not set" >>$(1); \
		fi; \
	else \
		echo "# CONFIG_DUMP_PREV_OOPS_MSG is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_JFFS_NVRAM/d" $(1); \
	if [ "$(JFFS_NVRAM)" = "y" ]; then \
                echo "CONFIG_JFFS_NVRAM=y" >>$(1); \
	else \
		echo "# CONFIG_JFFS_NVRAM is not set" >>$(1); \
        fi; \
	sed -i "/CONFIG_RTAC3200/d" $(1); \
	if [ "$(BUILD_NAME)" = "RT-AC3200" ]; then \
		echo "CONFIG_RTAC3200=y" >>$(1); \
	else \
		echo "# CONFIG_RTAC3200 is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_RTAC87U/d" $(1); \
	if [ "$(BUILD_NAME)" = "RT-AC87U" ]; then \
		echo "CONFIG_RTAC87U=y" >>$(1); \
	else \
		echo "# CONFIG_RTAC87U is not set" >>$(1); \
	fi; \
	sed -i "/CONFIG_RTAC88U/d" $(1); \
	if [ "$(BUILD_NAME)" = "RT-AC88U" ]; then \
		echo "CONFIG_RTAC88U=y" >>$(1); \
	else \
		echo "# CONFIG_RTAC88U is not set" >>$(1); \
	fi; \
	if [ "$(GMAC3)" = "y" ]; then \
		sed -i "/CONFIG_BCM_GMAC3/d" $(1); \
		echo "CONFIG_BCM_GMAC3=y" >>$(1); \
		sed -i "/CONFIG_BCM_FA/d" $(1); \
		echo "# CONFIG_BCM_FA is not set" >>$(1); \
	else \
		sed -i "/CONFIG_BCM_GMAC3/d" $(1); \
		echo "# CONFIG_BCM_GMAC3 is not set" >>$(1); \
		if [ "$(HND_ROUTER)" != "y" ]; then \
			sed -i "/CONFIG_BCM_FA/d" $(1); \
			echo "CONFIG_BCM_FA=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(NODHD)" = "y" ]; then \
		sed -i "/CONFIG_BCM_GMAC3=y/d" $(1); \
		echo "# CONFIG_BCM_GMAC3 is not set" >>$(1); \
		sed -i "/CONFIG_BCM_FA/d" $(1); \
		echo "CONFIG_BCM_FA=y" >>$(1); \
		sed -i "/CONFIG_DHDAP/d" $(1); \
		echo "# CONFIG_DHDAP is not set" >>$(1); \
		sed -i "/CONFIG_DPSTA/d" $(1); \
		echo "# CONFIG_DPSTA is not set" >>$(1); \
	fi; \
	if [ "$(DHDAP)" = "y" ]; then \
		if [ "$(HND_ROUTER)" != "y" ]; then \
			sed -i "/CONFIG_DHDAP/d" $(1); \
			echo "CONFIG_DHDAP=m" >>$(1); \
		fi; \
		sed -i "/CONFIG_WL=m/d" $(1); \
		echo "# CONFIG_WL is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_APSTA/d" $(1); \
		echo "# CONFIG_WL_USE_APSTA is not set" >>$(1); \
		sed -i "/CONFIG_WL_ALL_PASSIVE_RUNTIME/d" $(1); \
		sed -i "/CONFIG_WAPI/d" $(1); \
		echo "# CONFIG_WAPI is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_AP/d" $(1); \
		echo "# CONFIG_WL_USE_AP is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_AP_SDSTD/d" $(1); \
		echo "# CONFIG_WL_USE_AP_SDSTD is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_STA/d" $(1); \
		echo "# CONFIG_WL_USE_STA is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_AP_ONCHIP_G/d" $(1); \
		echo "# CONFIG_WL_USE_AP_ONCHIP_G is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_STA_ONCHIP_G/d" $(1); \
		echo "# CONFIG_WL_USE_STA_ONCHIP_G is not set" >>$(1); \
		sed -i "/CONFIG_WL_USE_APSTA_ONCHIP_G/d" $(1); \
		echo "# CONFIG_WL_USE_APSTA_ONCHIP_G is not set" >>$(1); \
		sed -i "/CONFIG_WL_ALL_PASSIVE_ON/d" $(1); \
		echo "# CONFIG_WL_ALL_PASSIVE_ON is not set" >>$(1); \
		sed -i "/CONFIG_DPSTA/d" $(1); \
		echo "# CONFIG_DPSTA is not set" >>$(1); \
		sed -i "/CONFIG_PLC/d" $(1); \
		echo "# CONFIG_PLC is not set" >>$(1); \
	else \
		sed -i "/CONFIG_DHDAP/d" $(1); \
		echo "# CONFIG_DHDAP is not set" >>$(1); \
	fi; \
	if [ "$(DPSTA)" = "y" ]; then \
		sed -i "/CONFIG_DPSTA/d" $(1); \
		echo "CONFIG_DPSTA=m" >>$(1); \
	fi; \
	sed -i "/CONFIG_LINUX_MTD/d" $(1); \
	if [ "$(LINUX_MTD)" = "" ]; then \
		echo "CONFIG_LINUX_MTD=32" >>$(1); \
	else \
		echo "CONFIG_LINUX_MTD=$(LINUX_MTD)" >>$(1); \
	fi; \
	sed -i "/CONFIG_NF_CONNTRACK_EVENTS/d" $(1); \
	if [ "$(BWDPI)" = "y" ] || [ "$(HND_ROUTER)" = "y" ]; then \
		echo "CONFIG_NF_CONNTRACK_EVENTS=y" >>$(1); \
	else \
		echo "# CONFIG_NF_CONNTRACK_EVENTS is not set" >>$(1); \
	fi; \
	if [ "$(BWDPI)" = "y" ]; then \
		sed -i "/CONFIG_NET_SCH_HTB/d" $(1); \
		echo "CONFIG_NET_SCH_HTB=y" >>$(1); \
		sed -i "/CONFIG_NET_SCH_SFQ/d" $(1); \
		echo "CONFIG_NET_SCH_SFQ=y" >>$(1); \
		sed -i "/CONFIG_CLS_U32_PERF/d" $(1); \
		echo "CONFIG_CLS_U32_PERF=y" >>$(1); \
		sed -i "/CONFIG_CLS_U32_MARK/d" $(1); \
		echo "CONFIG_CLS_U32_MARK=y" >>$(1); \
	fi; \
	if [ "$(USB_DEBUG)" = "y" ]; then \
		sed -i "/CONFIG_USB_DEBUG/d" $(1); \
		echo "CONFIG_USB_DEBUG=y" >>$(1); \
		sed -i "/CONFIG_DEBUG_FS/d" $(1); \
		echo "CONFIG_DEBUG_FS=y" >>$(1); \
	fi; \
	if [ "$(TFAT)" = "y" ]; then \
		sed -i "/CONFIG_MSDOS_FS/d" $(1); \
		echo "# CONFIG_MSDOS_FS is not set" >>$(1); \
		sed -i "/CONFIG_VFAT_FS/d" $(1); \
		echo "# CONFIG_VFAT_FS is not set" >>$(1); \
	fi; \
	if [ "$(HFS)" = "open" ]; then \
		sed -i "/CONFIG_HFS_FS/d" $(1); \
		echo "CONFIG_HFS_FS=y" >>$(1); \
		sed -i "/CONFIG_HFSPLUS_FS/d" $(1); \
		echo "CONFIG_HFSPLUS_FS=y" >>$(1); \
		sed -i "/CONFIG_MAC_PARTITION/d" $(1); \
		echo "CONFIG_MAC_PARTITION=y" >>$(1); \
	fi; \
	if [ "$(BLINK_LED)" = "y" ]; then \
		sed -i "/CONFIG_USB_BUS_STATS/d" $(1); \
		echo "CONFIG_USB_BUS_STATS=y" >>$(1); \
	else \
		sed -i "/CONFIG_USB_BUS_STATS/d" $(1); \
		echo "# CONFIG_USB_BUS_STATS is not set" >>$(1); \
	fi; \
	if [ "$(DEBUGFS)" = "y" ]; then \
		sed -i "/CONFIG_DEBUG_FS/d" $(1); \
		echo "CONFIG_DEBUG_FS=y" >>$(1); \
		sed -i "/CONFIG_USB_MON/d" $(1); \
		echo "CONFIG_USB_MON=m" >>$(1); \
		if [ "$(ARM)" = "y" ]; then \
			sed -i "/CONFIG_GCOV_KERNEL/d" $(1); \
			echo "# CONFIG_GCOV_KERNEL is not set" >>$(1); \
			sed -i "/CONFIG_L2TP_DEBUGFS/d" $(1); \
			echo "# CONFIG_L2TP_DEBUGFS is not set" >>$(1); \
			sed -i "/CONFIG_JBD_DEBUG/d" $(1); \
			echo "# CONFIG_JBD_DEBUG is not set" >>$(1); \
			sed -i "/CONFIG_JBD2_DEBUG/d" $(1); \
			echo "# CONFIG_JBD2_DEBUG is not set" >>$(1); \
			sed -i "/CONFIG_LKDTM/d" $(1); \
			echo "# CONFIG_LKDTM is not set" >>$(1); \
			sed -i "/CONFIG_DYNAMIC_DEBUG/d" $(1); \
			echo "# CONFIG_DYNAMIC_DEBUG is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(BONDING)" = "y" ]; then \
		sed -i "/CONFIG_BONDING/d" $(1); \
		echo "CONFIG_BONDING=m" >>$(1); \
	fi; \
	if [ "$(PLC_UTILS)" = "y" ]; then \
		sed -i "/CONFIG_MTD_JFFS2_PARTS/d" $(1); \
		echo "# CONFIG_MTD_JFFS2_PARTS is not set" >>$(1); \
		sed -i "/CONFIG_MTD_PLC_PARTS/d" $(1); \
		echo "CONFIG_MTD_PLC_PARTS=y" >>$(1); \
	fi; \
	if [ "$(BCM_7114)" = "y" ]; then \
		sed -i "/CONFIG_CR4_OFFLOAD/d" $(1); \
		echo "# CONFIG_CR4_OFFLOAD is not set" >>$(1); \
		sed -i "/CONFIG_PLAT_UART_CLOCKS/d" $(1); \
		echo "# CONFIG_PLAT_UART_CLOCKS is not set" >>$(1); \
	fi; \
	if [ "$(BCM9)" = "y" ]; then \
		sed -i "/CONFIG_CR4_OFFLOAD/d" $(1); \
		echo "# CONFIG_CR4_OFFLOAD is not set" >>$(1); \
		sed -i "/CONFIG_PLAT_MUX_CONSOLE_CCB/d" $(1); \
		echo "# CONFIG_PLAT_MUX_CONSOLE_CCB is not set" >>$(1); \
		sed -i "/CONFIG_PLAT_GPIOLIB/d" $(1); \
		echo "# CONFIG_PLAT_GPIOLIB is not set" >>$(1); \
		sed -i "/CONFIG_PLAT_UART_CLOCKS/d" $(1); \
		echo "# CONFIG_PLAT_UART_CLOCKS is not set" >>$(1); \
		sed -i "/CONFIG_GENERIC_GPIO/d" $(1); \
		echo "# CONFIG_GENERIC_GPIO is not set" >>$(1); \
		sed -i "/CONFIG_BCM_GMAC3/d" $(1); \
		echo "# CONFIG_BCM_GMAC3 is not set" >>$(1); \
		sed -i "/CONFIG_DHDAP/d" $(1); \
		echo "# CONFIG_DHDAP is not set" >>$(1); \
		sed -i "/CONFIG_YAFFS_FS/d" $(1); \
		echo "# CONFIG_YAFFS_FS is not set" >>$(1); \
	fi; \
	if [ "$(RTAC1200G)" = "y" ]; then \
		sed -i "/CONFIG_MTD_BRCMNAND/d" $(1); \
		echo "# CONFIG_MTD_BRCMNAND is not set" >>$(1); \
		sed -i "/CONFIG_MTD_BRCMNAND/d" $(1); \
		echo "# CONFIG_MTD_BRCMNAND is not set" >>$(1); \
		sed -i "/CONFIG_MTD_NFLASH/d" $(1); \
		echo "# CONFIG_MTD_NFLASH is not set" >>$(1); \
		sed -i "/CONFIG_MTD_NAND_ECC/d" $(1); \
		echo "# CONFIG_MTD_NAND_ECC is not set" >>$(1); \
		sed -i "/CONFIG_MTD_NAND/d" $(1); \
		echo "# CONFIG_MTD_NAND is not set" >>$(1); \
		sed -i "/CONFIG_MTD_NAND_IDS/d" $(1); \
		echo "# CONFIG_MTD_NAND_IDS is not set" >>$(1); \
		sed -i "/CONFIG_MTD_BCMCONF_PARTS/d" $(1); \
		echo "# CONFIG_MTD_BCMCONF_PARTS is not set" >>$(1); \
	fi; \
	if [ "$(RGMII_BCM_FA)" = "y" ]; then \
		sed -i "/CONFIG_RGMII_BCM_FA/d" $(1); \
		echo "CONFIG_RGMII_BCM_FA=y" >>$(1); \
	fi; \
	if [ "$(SWITCH2)" = "RTL8365MB" ]; then \
		sed -i "/CONFIG_RTL8370MB/d" $(1); \
		echo "# CONFIG_RTL8370MB is not set" >>$(1); \
		sed -i "/CONFIG_RTL8365MB/d" $(1); \
		echo "CONFIG_RTL8365MB=m" >>$(1); \
	else \
	if [ "$(SWITCH2)" = "RTL8370MB" ]; then \
		sed -i "/CONFIG_RTL8365MB/d" $(1); \
		echo "# CONFIG_RTL8365MB is not set" >>$(1); \
		sed -i "/CONFIG_RTL8370MB/d" $(1); \
		echo "CONFIG_RTL8370MB=m" >>$(1); \
	else \
	if [ "$(SWITCH2)" = "" ]; then \
		sed -i "/CONFIG_RTL8365MB/d" $(1); \
		echo "# CONFIG_RTL8365MB is not set" >>$(1); \
		sed -i "/CONFIG_RTL8370MB/d" $(1); \
		echo "# CONFIG_RTL8370MB is not set" >>$(1); \
	fi; \
	fi; \
	fi; \
	if [ "$(BCM_MMC)" = "y" ]; then \
		sed -i "/CONFIG_MMC/d" $(1); \
		echo "CONFIG_MMC=y" >>$(1); \
		sed -i "/CONFIG_MMC_BLOCK/d" $(1); \
		echo "CONFIG_MMC_BLOCK=y" >>$(1); \
		sed -i "/CONFIG_MMC_BLOCK_BOUNCE/d" $(1); \
		echo "CONFIG_MMC_BLOCK_BOUNCE=y" >>$(1); \
		sed -i "/CONFIG_MMC_TEST/d" $(1); \
		echo "# CONFIG_MMC_TEST is not set" >>$(1); \
		sed -i "/CONFIG_IWMC3200TOP/d" $(1); \
		echo "# CONFIG_IWMC3200TOP is not set" >>$(1); \
		sed -i "/CONFIG_MMC_DEBUG/d" $(1); \
		echo "CONFIG_MMC_DEBUG=y" >>$(1); \
		sed -i "/CONFIG_MMC_UNSAFE_RESUME/d" $(1); \
		echo "# CONFIG_MMC_UNSAFE_RESUME is not set" >>$(1); \
		sed -i "/CONFIG_SDIO_UART/d" $(1); \
		echo "# CONFIG_SDIO_UART is not set" >>$(1); \
		sed -i "/CONFIG_MMC_SDHCI/d" $(1); \
		echo "CONFIG_MMC_SDHCI=y" >>$(1); \
		sed -i "/CONFIG_MMC_SDHCI_PCI/d" $(1); \
		echo "CONFIG_MMC_SDHCI_PCI=y" >>$(1); \
		sed -i "/CONFIG_MMC_RICOH_MMC/d" $(1); \
		echo "# CONFIG_MMC_RICOH_MMC is not set" >>$(1); \
		sed -i "/CONFIG_MMC_SDHCI_PLTFM/d" $(1); \
		echo "# CONFIG_MMC_SDHCI_PLTFM is not set" >>$(1); \
		sed -i "/CONFIG_MMC_TIFM_SD/d" $(1); \
		echo "# CONFIG_MMC_TIFM_SD is not set" >>$(1); \
		sed -i "/CONFIG_MMC_CB710/d" $(1); \
		echo "# CONFIG_MMC_CB710 is not set" >>$(1); \
		sed -i "/CONFIG_MMC_VIA_SDMMC/d" $(1); \
		echo "# CONFIG_MMC_VIA_SDMMC is not set" >>$(1); \
		sed -i "/CONFIG_MMC_SDHCI_IO_ACCESSORS/d" $(1); \
		echo "CONFIG_MMC_SDHCI_IO_ACCESSORS=y" >>$(1); \
	fi; \
	if [ "$(HND_ROUTER)" != "y" ]; then \
		sed -i "/CONFIG_BCM_RECVFILE/d" $(1); \
		if [ "$(BCM_RECVFILE)" = "y" ]; then \
			echo "CONFIG_BCM_RECVFILE=y" >>$(1); \
		else \
			echo "# CONFIG_BCM_RECVFILE is not set" >>$(1); \
		fi; \
	fi; \
	if [ "$(IPSEC)" = "y" ] || \
	   [ "$(IPSEC)" = "QUICKSEC" ] || \
	   [ "$(IPSEC)" = "STRONGSWAN" ]; then \
		sed -i "/CONFIG_XFRM is not set/d" $(1); \
		echo "CONFIG_XFRM=y" >>$(1); \
		sed -i "/CONFIG_XFRM_USER is not set/d" $(1); \
		echo "CONFIG_XFRM_USER=m" >>$(1); \
		sed -i "/CONFIG_NET_KEY is not set/d" $(1); \
		echo "CONFIG_NET_KEY=y" >>$(1); \
		sed -i "/ CONFIG_NETFILTER_XT_MATCH_POLICY is not set/d" $(1); \
		echo "CONFIG_NETFILTER_XT_MATCH_POLICY=y" >>$(1); \
		sed -i "/CONFIG_IP_ROUTE_VERBOSE is not set/d" $(1); \
		echo "CONFIG_IP_ROUTE_VERBOSE=y" >>$(1); \
		sed -i "/CONFIG_INET is not set/d" $(1); \
		echo "CONFIG_INET=y" >>$(1); \
		sed -i "/CONFIG_INET_AH is not set/d" $(1); \
		echo "CONFIG_INET_AH=m" >>$(1); \
		sed -i "/CONFIG_INET_ESP is not set/d" $(1); \
		echo "CONFIG_INET_ESP=m" >>$(1); \
		sed -i "/CONFIG_INET_IPCOMP is not set/d" $(1); \
		echo "CONFIG_INET_IPCOMP=m" >>$(1); \
		sed -i "/CONFIG_INET_XFRM_TUNNEL is not set/d" $(1); \
		echo "CONFIG_INET_XFRM_TUNNEL=y" >>$(1); \
		sed -i "/CONFIG_INET_TUNNEL is not set/d" $(1); \
		echo "CONFIG_INET_TUNNEL=y" >>$(1); \
		sed -i "/CONFIG_INET_XFRM_MODE_TRANSPORT is not set/d" $(1); \
		echo "CONFIG_INET_XFRM_MODE_TRANSPORT=y" >>$(1); \
		sed -i "/CONFIG_INET_XFRM_MODE_TUNNEL is not set/d" $(1); \
		echo "CONFIG_INET_XFRM_MODE_TUNNEL=y" >>$(1); \
		sed -i "/CONFIG_NETFILTER is not set/d" $(1); \
		echo "CONFIG_NETFILTER=y" >>$(1); \
		sed -i "/CONFIG_NETFILTER_XTABLES is not set/d" $(1); \
		echo "CCONFIG_NETFILTER_XTABLES=y" >>$(1); \
		sed -i "/CONFIG_NETFILTER_XT_MATCH_POLICY is not set/d" $(1); \
		echo "CONFIG_NETFILTER_XT_MATCH_POLICY=y" >>$(1); \
		if [ "$(IPV6SUPP)" = "y" ]; then \
			sed -i "/CONFIG_INET6_AH is not set/d" $(1); \
			echo "CONFIG_INET6_AH=y" >>$(1); \
			sed -i "/CONFIG_INET6_ESP is not set/d" $(1); \
			echo "CONFIG_INET6_ESP=y" >>$(1); \
			sed -i "/CONFIG_INET6_IPCOMP is not set/d" $(1); \
			echo "CONFIG_INET6_IPCOMP=y" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_TUNNEL is not set/d" $(1); \
			echo "CONFIG_INET6_XFRM_TUNNEL=y" >>$(1); \
			sed -i "/CONFIG_INET6_TUNNEL is not set/d" $(1); \
			echo "CONFIG_INET6_TUNNEL=y" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_MODE_TRANSPORT is not set/d" $(1); \
			echo "CONFIG_INET6_XFRM_MODE_TRANSPORT=y" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_MODE_TUNNEL is not set/d" $(1); \
			echo "CONFIG_INET6_XFRM_MODE_TUNNEL=y" >>$(1); \
			sed -i "/CONFIG_IPV6_MULTIPLE_TABLES is not set/d" $(1); \
			echo "CONFIG_IPV6_MULTIPLE_TABLES=y" >>$(1); \
			sed -i "/CONFIG_INET_XFRM_MODE_BEET is not set/d" $(1); \
			echo "CONFIG_INET_XFRM_MODE_BEET=y" >>$(1); \
			sed -i "/CONFIG_INET6_XFRM_MODE_BEET is not set/d" $(1); \
			echo "CONFIG_INET6_XFRM_MODE_BEET=y" >>$(1); \
		fi; \
		sed -i "/CONFIG_CRYPTO_NULL is not set/d" $(1); \
		echo "CONFIG_CRYPTO_NULL=y" >>$(1); \
		sed -i "/CONFIG_CRYPTO_SHA256 is not set/d" $(1); \
		echo "CONFIG_CRYPTO_SHA256=y" >>$(1); \
		sed -i "/CONFIG_CRYPTO_SHA512 is not set/d" $(1); \
		echo "CONFIG_CRYPTO_SHA512=y" >>$(1); \
	fi; \
	if [ "$(ARM)" = "y" ]; then \
		sed -i "/CONFIG_NETFILTER_TPROXY/d" $(1); \
		echo "CONFIG_NETFILTER_TPROXY=m" >>$(1); \
		sed -i "/CONFIG_NETFILTER_XT_MATCH_COMMENT/d" $(1); \
		echo "CONFIG_NETFILTER_XT_MATCH_COMMENT=m" >>$(1); \
		sed -i "/CONFIG_NETFILTER_XT_MATCH_SOCKET/d" $(1); \
		echo "CONFIG_NETFILTER_XT_MATCH_SOCKET=m" >>$(1); \
		sed -i "/CONFIG_NETFILTER_XT_TARGET_TPROXY/d" $(1); \
		echo "CONFIG_NETFILTER_XT_TARGET_TPROXY=m" >>$(1); \
	fi;\
	if [ "$(OPENVPN)" = "y" ]; then \
		sed -i "/CONFIG_TUN/d" $(1); \
		echo "CONFIG_TUN=m" >>$(1); \
	fi;\
	if [ "$(VPN_FUSION)" = "y" ]; then \
		sed -i "/CONFIG_PPP_MPPE/d" $(1); \
		echo "CONFIG_PPP_MPPE=y" >>$(1); \
		sed -i "/CONFIG_PPP_DEFLATE/d" $(1); \
		echo "CONFIG_PPP_DEFLATE=y" >>$(1); \
		sed -i "/CONFIG_PPP_BSDCOMP/d" $(1); \
		echo "CONFIG_PPP_BSDCOMP=y" >>$(1); \
		sed -i "/CONFIG_IP_NF_TARGET_ROUTE/d" $(1); \
		echo "CONFIG_IP_NF_TARGET_ROUTE=y" >>$(1); \
		sed -i "/CONFIG_PPP_SYNC_TTY is not set/d" $(1); \
		echo "CONFIG_PPP_SYNC_TTY=y" >>$(1); \
		sed -i "/CONFIG_PPP_MULTILINK/d" $(1); \
		echo "CONFIG_PPP_MULTILINK=y" >>$(1); \
	fi; \
	)
	$(call platformKernelConfig, $(1))
	$(call extraKernelConfig, $(1))
endef

mk-%:
	@$(MAKE) -C router $(shell echo $@ | sed s/mk-//)

bbconfig:
	@cp $(BUSYBOX_DIR)/config_base $(BUSYBOX_DIR)/config_$(lowercase_B)
	$(call BusyboxOptions, $(BUSYBOX_DIR)/config_$(lowercase_B))
	@cd $(BUSYBOX_DIR) && \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp config_current .config
	$(MAKE) -C router bboldconf
	@echo done

bin:
ifeq ($(BUILD_NAME),)
	@echo $@" is not a valid target!"
	@false
endif
ifeq ($(HND_ROUTER),y)
	@echo BRCM_BOARD_ID=$(BRCM_BOARD_ID)
	@rm -f bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP) && ln -sf impl51 bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)
	@rm -f $(PROFILE_FILE_PUB) && ln -sf $(PROFILE_FILE) $(PROFILE_FILE_PUB)
	@rm -f $(SRCBASE)/.config && ln -sf $(HND_SRC)/.config $(SRCBASE)/.config
	@rm -f $(LINUXDIR)/.config
	@echo '#define BBOARD_ID $(BOARD_ID)' > router/shared/bbid.h
endif
	@cp router/config_base router/config_$(lowercase_B)
	@cp $(BUSYBOX_DIR)/config_base $(BUSYBOX_DIR)/config_$(lowercase_B)

ifeq ($(REALTEK),y)
	@if [ "$(BUILD_NAME)" = "RP-AC53" ] ; then \
		cp $(RSDKDIR)/boards/rtl8881a/config.rtl819x.$(BUILD_NAME) $(RSDKDIR)/.config ; \
	elif [ "$(BUILD_NAME)" = "RP-AC68U" ] ; then \
		cp $(RSDKDIR)/boards/rtl8198C_8954E/config.rtl819x.$(BUILD_NAME) $(RSDKDIR)/.config ; \
	elif [ "$(BUILD_NAME)" = "RP-AC55" ] ; then \
		cp $(RSDKDIR)/boards/rtl8197F/config.rtl819x.$(BUILD_NAME) $(RSDKDIR)/.config ; \
	fi ;
	@make -C $(RSDKDIR) config ;
	@cp $(LINUXDIR)/.config $(LINUXDIR)/config_base ; \
	cp $(LINUXDIR)/config_base $(LINUXDIR)/config_$(lowercase_B)
else ifeq ($(BCMWL6A),y)
	@cp $(LINUXDIR)/config_base.6a $(LINUXDIR)/config_$(lowercase_B)
else
	@if [ "$(RALINK)" = "y" ]; then \
		if [ "$(BUILD_NAME)" = "RT-AC1200" ] || [ "$(BUILD_NAME)" = "RT-N600" ] ; then \
			cp $(LINUXDIR)/ralink/Kconfig_$(lowercase_B) $(LINUXDIR)/ralink/Kconfig ; \
			cp $(LINUXDIR)/config_base.MT7628 $(LINUXDIR)/config_$(lowercase_B) ; \
		elif [ "$(BUILD_NAME)" = "RT-N11P_B1" ] || [ "$(BUILD_NAME)" = "RT-N10P_V3" ] ; then \
			cp $(LINUXDIR)/ralink/Kconfig_rt-n11p_b1 $(LINUXDIR)/ralink/Kconfig ; \
			cp $(LINUXDIR)/config_base.rt-n11p_b1 $(LINUXDIR)/config_$(lowercase_B) ; \
		elif [ "$(BUILD_NAME)" = "RP-AC56" ]; then \
			cp $(LINUXDIR)/ralink/Kconfig_$(lowercase_B) $(LINUXDIR)/ralink/Kconfig ; \
			cp $(LINUXDIR)/config_base.MT7621 $(LINUXDIR)/config_$(lowercase_B) ; \
		elif [ "$(BUILD_NAME)" = "RT-AC1200GA1" ] || [ "$(BUILD_NAME)" = "RT-AC1200GU" ]; then \
			cp $(LINUXDIR)/ralink/Kconfig_$(lowercase_B) $(LINUXDIR)/ralink/Kconfig ; \
			cp $(LINUXDIR)/config_base.MT7621_ROUTER $(LINUXDIR)/config_$(lowercase_B) ; \
		elif [ "$(BUILD_NAME)" = "RT-AC51U+" ] ||[ "$(BUILD_NAME)" = "RT-AC53" ]; then \
			cp $(LINUXDIR)/ralink/Kconfig_$(lowercase_B) $(LINUXDIR)/ralink/Kconfig ; \
			cp $(LINUXDIR)/config_base.MT7620 $(LINUXDIR)/config_$(lowercase_B) ; \
		else \
			cp $(LINUXDIR)/config_base $(LINUXDIR)/config_$(lowercase_B) ; \
		fi ; \
	else \
		cp $(LINUXDIR)/config_base $(LINUXDIR)/config_$(lowercase_B) ; \
	fi ;
	@if [ -f $(LINUXDIR)/config_base_$(lowercase_B) ]; then \
		cp $(LINUXDIR)/config_base_$(lowercase_B) $(LINUXDIR)/config_$(lowercase_B); \
	fi
endif
	@echo "" >> router/config_$(lowercase_B)
	$(call RouterOptions, router/config_$(lowercase_B))
	$(call KernelConfig, $(LINUXDIR)/config_$(lowercase_B))
ifeq ($(REALTEK),y)
	$(call RtlFlashMpping, router/shared/sysdeps/realtek/rtl_flashmapping.h)
endif
	$(call BusyboxOptions, $(BUSYBOX_DIR)/config_$(lowercase_B))
ifeq ($(CONFIG_RALINK),y)
	@if [ "$(BUILD_NAME)" = "RT-N56UB1" ] || [ "$(BUILD_NAME)" = "RT-N56UB2" ]  ; then \
		if [ "$(MT7603_EXTERNAL_PA_EXTERNAL_LNA)" = "y" ] ; then \
			echo "build epa+elna fw" ; \
			gcc -DCONFIG_MT7603E_EXTERNAL_PA_EXTERNAL_LNA -g $(LINUXDIR)/drivers/net/wireless/rlt_wifi_7603E/tools/bin2h.c \
			-o $(LINUXDIR)/drivers/net/wireless/rlt_wifi_7603E/tools/bin2h ; \
		elif [ "$(MT7603_INTERNAL_PA_EXTERNAL_LNA)" = "y" ] ; then \
			echo "build ipa+elna fw" ; \
			gcc -DCONFIG_MT7603E_INTERNAL_PA_EXTERNAL_LNA -g $(LINUXDIR)/drivers/net/wireless/rlt_wifi_7603E/tools/bin2h.c \
			-o $(LINUXDIR)/drivers/net/wireless/rlt_wifi_7603E/tools/bin2h ; \
		fi ; \
		$(MAKE) -C $(LINUXDIR)/drivers/net/wireless/rlt_wifi_7603E build_e2fw; \
	fi
endif


	@$(MAKE) setprofile
	$(MAKE) all

ifeq ($(REALTEK),y)
define RtlFlashMpping
	@( \
		echo "#define CONFIG_RTL_HW_SETTING_OFFSET `sed -n 's/CONFIG_RTL_HW_SETTING_OFFSET=//p' $(LINUXDIR)/.config`" >$(1); \
		echo "#define CONFIG_RTL_LINUX_IMAGE_OFFSET `sed -n 's/CONFIG_RTL_LINUX_IMAGE_OFFSET=//p' $(LINUXDIR)/.config`" >>$(1); \
		if grep -q "CONFIG_MTD_NAND=y" $(LINUXDIR)/.config ; then \
			echo "#define CONFIG_MTD_NAND" >>$(1); \
		fi; \
		if grep -q "CONFIG_ASUS_DUAL_IMAGE_ENABLE=y" $(LINUXDIR)/.config ; then \
			echo "#define CONFIG_ASUS_DUAL_IMAGE_ENABLE" >>$(1); \
		fi; \
	)
endef
endif

define save_src_config
	@if [ -f .config ] ; then \
		if [ $(shell echo $(1) | grep -i "^RT4G-") ] ; then \
			NEW_BUILD_NAME=$(shell echo $(1) | tr a-z A-Z | sed 's/^RT//') ; \
		else \
			NEW_BUILD_NAME=$(shell echo $(1) | tr a-z A-Z) ; \
		fi ; \
		echo "CONFIGURED MODEL: $(BUILD_NAME)" ; \
		echo "SPECIFIED  MODEL: $${NEW_BUILD_NAME}" ; \
		echo "----------------------------------------------------------------------------" ; \
		if [ "$(BUILD_NAME)" != "$${NEW_BUILD_NAME}" ] ; then \
			echo "!!! MODEL NAME MISMATCH.  REMOVE .config AND MAKE AGAIN. !!!" ; \
			exit 1; \
		fi ; \
	fi ;
	@if [ -z '$($(shell echo $(1) | tr a-z A-Z))' ] ; then \
		echo NO THIS TARGET $(1) ; exit 1; \
	fi ;
	@if [ -f .config ] ; then \
			echo "Clean old model configuration"; \
			while read line ; do \
				var=`echo "$${line}"|sed -e "s,^export[       ]*,," -e "s,=.*$$,,"` ; \
				unset "$${var}" ; \
			done < .config; \
			echo "Update model configuration" ; \
			rm -f .config ; \
	fi ;
	@for var in $($(shell echo $(1) | tr a-z A-Z)) ; do \
		echo "export $${var}" >> .config ; \
		export $${var} ; \
	done ;
	@chmod 666 .config;

	@echo "";
endef

4g-% 4G-%:
	$(call save_src_config, RT$@)
	$(MAKE) bin

brt-% BRT-% rt-% RT-% gt-% GT-% rp-% RP-% ea-% EA-% tm-% TM-% pl-% PL-% ac% map-% MAP-% vzw-% VZW-%:
ifeq ($(REALTEK),y)
	rm -f include
	ln -sf include-n66u include
	if [ -e ./linux/rtl819x ]; then\
		rm ./linux/rtl819x;\
	fi;
	if [ -e ./linux/realtek/rtl819x ]; then\
		rm ./linux/realtek/rtl819x;\
	fi;
	if [ "$(shell echo $@ | tr a-z A-Z)" = "RP-AC55" ]; then\
		ln -sf ./realtek/rtl819x_97f ./linux/rtl819x;\
		ln -sf ./rtl819x_97f ./linux/realtek/rtl819x;\
	else \
		ln -sf ./realtek/rtl819x_98c ./linux/rtl819x;\
		ln -sf ./rtl819x_98c ./linux/realtek/rtl819x;\
	fi;
endif
	$(call save_src_config, $@)
	$(MAKE) bin

Bluecave bluecave:
	$(call save_src_config, $@)
	$(MAKE) bin

dsl-% DSL-%:
	$(call dsl_genbintrx_prolog)
	$(call save_src_config, $@)
	@$(MAKE) bin

ifeq ($(ALPINE)$(LANTIQ),y)
setprofile: prepare_toolchain kernel_patch
else
setprofile:
endif
	@echo ""
	@echo "Using $(N) profile, $(B) build config."
	@echo ""

	@cd $(LINUXDIR) ; \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp -f config_current .config

	@cd $(BUSYBOX_DIR) && \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp config_current .config

	@cd router ; \
		rm -f config_current ; \
		ln -s config_$(lowercase_B) config_current ; \
		cp config_current .config

	@if grep -q "CONFIG_RT3352_INIC_MII=m" $(LINUXDIR)/.config ; then \
		sed -i "/RTCONFIG_WLMODULE_RT3352_INIC_MII/d" router/.config; \
		echo "RTCONFIG_WLMODULE_RT3352_INIC_MII=y" >> router/.config; \
	fi
	@if grep -q "CONFIG_RTPCI_AP=m" $(LINUXDIR)/.config ; then \
		sed -i "/RTCONFIG_WLMODULE_RT3090_AP/d" router/.config; \
		echo "RTCONFIG_WLMODULE_RT3090_AP=y" >> router/.config; \
	fi
	@if grep -q "CONFIG_MT7610_AP=m" $(LINUXDIR)/.config ; then \
		sed -i "/RTCONFIG_WLMODULE_MT7610_AP/d" router/.config; \
		echo "RTCONFIG_WLMODULE_MT7610_AP=y" >> router/.config; \
	fi

	@if grep -q "CONFIG_RLT_WIFI=m" $(LINUXDIR)/.config ; then \
		sed -i "/RTCONFIG_WLMODULE_RLT_WIFI/d" router/.config; \
		echo "RTCONFIG_WLMODULE_RLT_WIFI=y" >> router/.config; \
	fi
	@if grep -q "CONFIG_WIFI_MT7603E=m" $(LINUXDIR)/.config ; then \
		sed -i "/RTCONFIG_WLMODULE_MT7603E_AP/d" router/.config; \
		echo "RTCONFIG_WLMODULE_MT7603E_AP=y" >> router/.config; \
	fi

	@if grep -q "CONFIG_RALINK_MT7628=y" $(LINUXDIR)/.config ; then \
		sed -i "/RTCONFIG_WLMODULE_MT7628_AP/d" router/.config; \
		echo "RTCONFIG_WLMODULE_MT7628_AP=y" >> router/.config; \
	fi

	@if grep -q "CONFIG_CHIP_MT7615E=y" $(LINUXDIR)/.config ; then \
		sed -i "/RTCONFIG_WLMODULE_MT7615E_AP/d" router/.config; \
		echo "RTCONFIG_WLMODULE_MT7615E_AP=y" >> router/.config; \
	fi

ifeq ($(HND_ROUTER),y)
	CURRENT_ARCH=$(KERNEL_ARCH) TOOLCHAIN_TOP= $(MAKE) prek
endif
	$(MAKE) -C router oldconfig

cleanlibc:
#	@$(MAKE) -C ../../tools-src/uClibc clean

libc: cleanlibc
#	@$(MAKE) -C ../../tools-src/uClibc
#	@$(MAKE) -C ../../tools-src/uClibc install

help:
	@echo "make [model id]"
	@echo "make mk-[package]"
	@echo "..etc..      other build configs"
	@echo "clean        -C router clean"
	@echo "cleanimage   rm -rf image"
	@echo "cleantools   clean btools, mksquashfs"
	@echo "cleankernel  -C Linux distclean (but preserves .config)"
	@echo "distclean    distclean of Linux & busybox (but preserve .configs)"
	@echo "prepk        -C Linux oldconfig dep"
	@echo "libc         -C uClibc clean, all, install"

kernel-tags: dummy
	$(MAKE) -C $(LINUXDIR) tags

tags: kernel-tags
	ctags -R $(CTAGS_EXCLUDE_OPT) $(CTAGS_DEFAULT_DIRS)

stage-tags:
	$(MAKE) -C router stage-tags

cscope: dummy
	cscope -bkR -s `echo "$(CTAGS_DEFAULT_DIRS)" | sed -e "s, , -s ,g"` -s $(SRC_ROOT)/router

all-tags: kernel-tags
	ctags -R shared $(CTAGS_EXCLUDE_OPT) $(SRC_ROOT)/router

clean-tags: dummy
	$(RM) -f $(LINUXDIR)/tags tags

clean-cscope: dummy
	$(RM) -f $(LINUXDIR)/cscope.* cscope.*

install gen_target:
ifeq ($(RTCONFIG_REALTEK),y)
	@$(MAKE) -C router gen_kernelrelease
endif
ifneq ($(PLATFORM_ROUTER),)
	$(MAKE) -C $(PLATFORM_ROUTER) $@
endif
	$(MAKE) -C router $@

gen_prebuilt:
	-mkdir -p $(PBDIR)
	$(MAKE) -f upb.mak PBDIR=${PBDIR}
	@if [ -f .gpl_excludes_router ]; then cp -f .gpl_excludes_router ${PBDIR}/release/.; fi

#
# Generic rules for platform specific software packages.
#

ifneq ($(PLATFORM_ROUTER),)
$(PLATFORM_ROUTER)/%: dummy
	@[ ! -d $(PLATFORM_ROUTER)/$* ] || $(MAKE) -C $(PLATFORM_ROUTER) $*

$(PLATFORM_ROUTER)/%-clean: dummy
	@-[ ! -d $(PLATFORM_ROUTER)/$* ] || $(MAKE) -C $(PLATFORM_ROUTER) $*-clean

$(PLATFORM_ROUTER)/%-install: dummy
	@[ ! -d $(PLATFORM_ROUTER)/$* ] || $(MAKE) -C $(PLATFORM_ROUTER) $* $*-install

$(PLATFORM_ROUTER)/%-stage: dummy
	@[ ! -d $(PLATFORM_ROUTER)/$* ] || $(MAKE) -C $(PLATFORM_ROUTER) $* $*-stage

$(PLATFORM_ROUTER)/%-build: dummy
	$(MAKE) $(PLATFORM_ROUTER)/$*-clean $(PLATFORM_ROUTER)/$*

$(PLATFORM_ROUTER)/%-tags: dummy
	[ ! -d $(PLATFORM_ROUTER)/$* ] || ctags -a -R $(CTAGS_EXCLUDE_OPT) $(PLATFORM_ROUTER)/$*
endif

#
# Generic rules
#

%: dummy
	@[ ! -d router/$* ] || $(MAKE) -C router $@


%-clean: dummy
	@-[ ! -d router/$* ] || $(MAKE) -C router $@

%-install: dummy
	@[ ! -d router/$* ] || $(MAKE) -C router $* $@

%-stage: dummy
	@[ ! -d router/$* ] || $(MAKE) -C router $* $@

%-build: dummy
	$(MAKE) $*-clean $*

%-tags: dummy
	@[ ! -d router/$* ] || ctags -a -R $(CTAGS_EXCLUDE_OPT) $(SRC_ROOT)/router/$*

#
# check extendno
#

get_extendno:
	@if [ "$(ID)" == "" ]; then echo "No ID is assigned"; exit 1; fi
	git log --pretty=oneline asuswrt_$(KERNEL_VER).$(FS_VER).$(SERIALNO)..$(ID) | wc -l
	@echo $(RC_EXT_NO)
	@echo $(EXTENDNO)

.PHONY: all clean distclean cleanimage cleantools cleankernel prepk what setprofile libc help image default bin_file
.PHONY: a b c d m Makefile allversions
.PHONY: tags
.PHONY: dummy

