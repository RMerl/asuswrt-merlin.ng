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
# $Id: Makefile 693762 2017-04-05 02:06:54Z $
#

ifneq ($(strip $(BUILD_HND_NIC)),)
$(warning hnd_wl/Makefile BUILD_HND_NIC IS set)
SRCBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src
else
SRCBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src
endif

ROUTERBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/components/router

TARGET := wl
WLCONF_H := wlconf.h
WLCONF_O := wlconf.o

ifeq ($(RTCONFIG_BRCM_HOSTAPD),y)
export CONFIG_BCM_HOSTAPD=y
endif

ifeq ($(CONFIG_BUZZZ_FUNC),y)
subdir-ccflags-y += -O0 -fno-inline -finstrument-functions
endif # CONFIG_BUZZZ_FUNC

ifneq ($(BUILD_HND_MFG),)
export BUILD_MFG := 1
export WLTEST := 1
endif

ifeq ($(CONFIG_BCM_AIRIQ),y)
export CONFIG_AIRIQ=y
endif

ifeq ($(CONFIG_BCM_HND_EAP),y)
# EAP default
CONFIG_WL_USE_APSTA_EAP=y
ifneq ($(BUILD_HND_EAP_AP1),)
CONFIG_WL_APSTA_EAP := "wlconfig_lx_router_apsta_eap_ap1"
else
CONFIG_WL_APSTA_EAP := "wlconfig_lx_router_apsta_eap"
endif
CONFIG_WL_CONF := $(CONFIG_WL_APSTA_EAP)
else
# APSTA default for rest
CONFIG_WL_USE_APSTA=y
ifneq ($(CMWIFI),)
CONFIG_WL_APSTA = "wlconfig_lx_cmwifi_apsta"
else
CONFIG_WL_APSTA="wlconfig_lx_router_apsta"
endif
CONFIG_WL_CONF := $(CONFIG_WL_APSTA)
# add additional wl variants here otherthan defaults.
# VLIST_BASE := AP
endif

VLIST          += $(VLIST_BASE)

# For PREBUILD and postbuild w/WLSRC, compile variants in subdirs
# Subdir names use lowercase versions of the variant suffixes
#wlsubdirs := $(addprefix wl_,$(foreach v,$(VLIST),$(shell echo "$(v)/" | tr A-Z a-z)))
wlsubdirs := $(addprefix wl_,$(foreach v,$(VLIST),$(shell echo "$(v)" | tr A-Z a-z)))

-include $(src)/$(ROUTERBASE_OFFSET)/hnd/Makefile.shared

$(info "wlsubdirs: $(wlsubdirs)")

#subdir-y += $(wlsubdirs)
#subdir-m += $(wlsubdirs)

.NOTPARALLEL:
include $(src)/wl_generic.mk
