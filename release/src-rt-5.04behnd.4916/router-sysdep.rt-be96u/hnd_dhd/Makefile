#
# Makefile for the Broadcom wl driver
#
# Copyright (C) 2023, Broadcom. All Rights Reserved.
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

# bcmdrivers/broadcom/net/wl/impl101/sys/src/dhd
ifeq ($(PREBUILT_EXTRAMOD),1)
REBUILD_DHD_MODULE=0
else
REBUILD_DHD_MODULE=$(shell if [ -d "$(SRCBASE)/../../$(SRCBASE_DHD)/dhd/sys" ]; then echo 1; else echo 0; fi)
endif
$(info "module : $(REBUILD_DHD_MODULE)")

ifneq ($(wildcard $(src)/../../router-sysdep/hnd_dhd/prebuilt/dhd.o),)
REBUILD_DHD_MODULE=0
endif

# this flag scope only for DHD module, the flag defined in wl.mk has different scope
# similar to WL_MONITOR if the tar ball doesn't have event_log.h, disable the event log feature
BCM_EVENT_LOG = $(shell if [ -f "$(SRCBASE)/../../$(SRCBASE_DHD)/shared/event_log.h" ]; then echo 1; else echo 0; fi)
#ifdef CMWIFI
ifneq ($(CMWIFI),)
ifeq ($(CMWIFI_33940),)
KBUILD_EXTRA_SYMBOLS = $(ALLSRCBASE)/$(HNDDRIVER_BASE)/hnd/Module.symvers
KBUILD_EXTRA_SYMBOLS += $(ALLSRCBASE)/$(HNDDRIVER_BASE)/hnd_emf/Module.symvers
KBUILD_EXTRA_SYMBOLS += $(ALLSRCBASE)/$(HNDDRIVER_BASE)/hnd_igs/Module.symvers
KBUILD_EXTRA_SYMBOLS += $(WL_CMWIFI_DIR)/mods/wlcsm_ext/Module.symvers
endif
ifneq ($(CMWIFI_EROUTER),)
    EXTRA_CFLAGS += -I$(WL_CMWIFI_DIR)/mods/wlerouter/include
    EXTRA_CFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/fpm
endif
endif
#endif

# Linux 5.3 and beyond set -Wimplicit-fallthrough=5, which
# ceases to match fall-through comments to suppress the
# warning/error. The only way to is to add the fallthrough
# attribute which is defined as the "fallthrough" macro in
# the kernel. However, the WLAN code compiles against many
# kernels (some very old), with many compilers (some very
# old). Until all these old things are deprecated, for all
# WLAN modules, we set -Wimplicit-fallthrough=3 rather than
# 5, allowing fall-through comments.
KBUILD_CFLAGS := $(subst -Wimplicit-fallthrough=5,-Wimplicit-fallthrough=3,$(KBUILD_CFLAGS))

ifneq ($(CM_BUILDROOT),)
KBUILD_CFLAGS += -DCM_BUILDROOT
endif

# If source directory (dhd/src/dhd/sys) exists then build dhd, otherwise use pre-builts
ifeq ($(REBUILD_DHD_MODULE),1)

    ifneq ($(BUILD_HND_MFG),)
        export BUILD_MFG := 1
        export WLTEST := 1
    endif
    ifeq ($(RTCONFIG_BRCM_HOSTAPD),y)
        export CONFIG_BCM_HOSTAPD=y
    endif

    # Include router config to source LBR settings
    ifeq ($(CM_BUILDROOT),)
       -include $(src)/../.config
    else
       include $(SRCBASE_ROUTER)/.config
    endif

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
    KBUILD_CFLAGS += -DBCM_ROUTER
    KBUILD_CFLAGS += -DBCM_DHDHDR
    KBUILD_CFLAGS += -DBCMHWA

    # dhd-objs is for linking to dhd.o
    $(TARGET)-objs := $(DHD_OBJS)

    MODEL = $(subst -,,$(BUILD_NAME))
    KBUILD_CFLAGS += -D$(MODEL)

else # Use prebuilt modules

    prebuilt := dhd.o
    $(TARGET)-objs := prebuilt/$(prebuilt)
    obj-$(CONFIG_BCM_WLAN) := $(TARGET).o

endif
