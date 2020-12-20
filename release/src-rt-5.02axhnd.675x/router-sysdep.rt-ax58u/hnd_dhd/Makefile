#
# Makefile for the Broadcom wl driver
#
# Copyright (C) 2020, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id: Makefile 692718 2017-03-29 03:25:07Z $
#

include $(SRCBASE)/.config
include $(HND_SRC)/router/.config

TARGET := dhd
obj-$(CONFIG_BCM_WLAN) := $(TARGET).o

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
ifndef SRCBASE_DHD
    SRCBASE_DHD := sys/src
else
    SRCBASE_DHD := $(subst $(dir $(patsubst %/,%,$(dir $(SRCBASE_DHD)))),,$(SRCBASE_DHD))
endif

SRCBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/$(SRCBASE_DHD)
SRCBASE := $(src)/$(SRCBASE_OFFSET)
COMPONENTSBASE_OFFSET := ../../../components
COMPONENTSBASE := $(src)/$(COMPONENTSBASE_OFFSET)
SRCBASE_ROUTER := $(COMPONENTSBASE)/router-sysdep

DHDMAKEFILE    := $(SRCBASE)/../../$(SRCBASE_DHD)/dhd/config/dhd_router.mk

# bcmdrivers/broadcom/net/wl/impl55/sys/src/dhd
ifeq ($(PREBUILT_EXTRAMOD),1)
REBUILD_DHD_MODULE=0
else
REBUILD_DHD_MODULE=$(shell if [ -d "$(SRCBASE)/../../$(SRCBASE_DHD)/dhd/sys" ]; then echo 1; else echo 0; fi)
endif
$(info "module : $(REBUILD_DHD_MODULE)")

ifneq ($(wildcard $(src)/../../router-sysdep/hnd_dhd/prebuilt/dhd.o),)
REBUILD_DHD_MODULE=0
endif

# XXX: d11.h is required to support WL_MONITOR in dhd. If this file is not
# present in release tarball, disable WL_MONITOR to avoid build problems.
WL_MONITOR=$(shell if [ -f "$(SRCBASE)/../../$(SRCBASE_DHD)/include/hndd11.h" ]; then echo 1; else echo 0; fi)

# If source directory (dhd/src/dhd/sys) exists then build dhd, otherwise use pre-builts
ifeq ($(REBUILD_DHD_MODULE),1)

    ifeq ($(CONFIG_BUZZZ_FUNC),y)
        subdir-ccflags-y += -O0 -fno-inline -finstrument-functions
    endif # CONFIG_BUZZZ_FUNC

    ifneq ($(BUILD_HND_MFG),)
        export BUILD_MFG := 1
        export WLTEST := 1
    endif
    ifeq ($(RTCONFIG_BRCM_HOSTAPD),y)
        export CONFIG_BCM_HOSTAPD=y
    endif

    # Include router config to source LBR settings
    -include $(src)/../.config

    # Include master configuration from dhd makefile
    include $(DHDMAKEFILE)

    # Fix the include paths for DHD. DHD gets built from a
    # different source tree.
    KBUILD_CFLAGS_REMOVE += $(WLAN_ComponentIncPathA)
    KBUILD_CFLAGS_REMOVE += $(WLAN_StdIncPathA)
    KBUILD_CFLAGS_REMOVE += -Werror=date-time

    KBUILD_CFLAGS   := $(call uniq,$(filter-out $(KBUILD_CFLAGS_REMOVE), $(KBUILD_CFLAGS)))

    # BCA build identifier
    KBUILD_CFLAGS += -DBCA_HNDROUTER
    KBUILD_CFLAGS += -DBCM_DHDHDR
    KBUILD_CFLAGS += -DBCMHWA

    # dhd-objs is for linking to dhd.o
    $(TARGET)-objs := $(DHD_OBJS)

ifeq ($(CONFIG_ARM64),)
# 32 bits NIC driver size reduction, makes use of gcc/ld unused symbol collect capability
KBUILD_CFLAGS += -fdata-sections -ffunction-sections
TMP_STR := --gc-sections --print-gc-sections --entry=fake_main --script=$(SRCBASE)/shared/linux.module.arm.lds
ifeq ($(CMWIFI),)
LDFLAGS += $(TMP_STR)
else
# CMWIFI builds use a later Linux kernel (4.9) than DSL builds (4.1)
KBUILD_LDFLAGS_MODULE += $(TMP_STR)
endif
endif

    MODEL = $(subst -,,$(BUILD_NAME))
    KBUILD_CFLAGS += -D$(MODEL)

else # Use prebuilt modules

    prebuilt := dhd.o
    $(TARGET)-objs := prebuilt/$(prebuilt)
    obj-$(CONFIG_BCM_WLAN) := $(TARGET).o

endif
