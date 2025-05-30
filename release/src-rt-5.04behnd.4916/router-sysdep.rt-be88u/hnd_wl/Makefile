#
# Makefile for the Broadcom wl driver
#
# Copyright (C) 2024, Broadcom. All Rights Reserved.
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

#ifdef CMWIFI
ifneq ($(CMWIFI),)
ccflags-y += -Wno-error=date-time
#WL_PHYLIB := 1
KBUILD_EXTRA_SYMBOLS = $(ALLSRCBASE)/$(HNDDRIVER_BASE)/hnd/Module.symvers
KBUILD_EXTRA_SYMBOLS += $(ALLSRCBASE)/$(HNDDRIVER_BASE)/hnd_emf/Module.symvers
KBUILD_EXTRA_SYMBOLS += $(ALLSRCBASE)/$(HNDDRIVER_BASE)/hnd_igs/Module.symvers
KBUILD_EXTRA_SYMBOLS += $(WL_CMWIFI_DIR)/mods/wlcsm_ext/Module.symvers

KBUILD_CFLAGS += -DBCMDRIVER -DCMWIFI
KBUILD_CFLAGS += -DBCMDBG_ERR
ifneq ($(CMWIFI_EROUTER),)
    EXTRA_CFLAGS += -I$(WL_CMWIFI_DIR)/mods/wlerouter/include
    EXTRA_CFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/fpm
endif
endif # CMWIFI
#endif

ifneq ($(CM_BUILDROOT),)
KBUILD_CFLAGS += -DCM_BUILDROOT
endif

ifneq ($(strip $(KTOOLCHAIN_SYSROOT)),)
SYSROOT_CC :=$(CC) --sysroot=$(KTOOLCHAIN_SYSROOT)
CC := $(SYSROOT_CC)
endif

ifneq ($(strip $(BUILD_HND_NIC)),)
$(warning hnd_wl/Makefile BUILD_HND_NIC IS set)
SRCBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src
else
SRCBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src
endif

ROUTERBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/components/router-sysdep

TARGET := wl
WLCONF_H := wlconf.h
WLCONF_O := wlconf.o

$(shell echo "# auto generated" > $(HND_SRC)/.config_hnd_wl)

ifeq ($(RTCONFIG_BRCM_HOSTAPD),y)
export CONFIG_BCM_HOSTAPD=y
endif

ifneq ($(BUILD_HND_MFG),)
export BUILD_MFG := 1
export WLTEST := 1
else
ifeq ($(BUILD_BCM_WLMLO),y)
export CONFIG_NIC_MLO := 1
endif
endif

ifeq ($(CONFIG_BCM_AIRIQ),y)
export CONFIG_AIRIQ=y
$(shell echo "CONFIG_AIRIQ=y" >> $(HND_SRC)/.config_hnd_wl)
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

#ifdef CMWIFI
ifneq ($(CMWIFI),)
wlsubdirs :=
endif
#endif

-include $(src)/$(ROUTERBASE_OFFSET)/hnd/Makefile.shared

$(info "wlsubdirs: $(wlsubdirs)")

#subdir-y += $(wlsubdirs)
#subdir-m += $(wlsubdirs)

.NOTPARALLEL:
include $(src)/wl_generic.mk

ifneq ($(CMWIFI)$(CONFIG_BCM_WLAN_RDKB),)
$(info ### Writing $(TARGET)-objs to $(src)/wlobjs.tmp ###)
$(shell echo "$(addprefix $(src)/,$($(TARGET)-objs))" > $(src)/wlobjs.tmp)

# Copied from cmd_link_multi-y in linux/scripts/Makefile.build, replace $(link_multi_deps) by @$(src)/wlobjs.tmp
override cmd_link_multi-m = $(LD) $(ld_flags) -r -o $@ $(cmd_secanalysis) @$(src)/wlobjs.tmp

# Linux 5.4 uses cmd_mod to generate *.mod, in this command abs path $(obj)/ is addprefix to each *.o.
# Replace it by wlobjs.tmp.
override cmd_mod = cat $(src)/wlobjs.tmp > $@; $(cmd_undef_syms) > $@

endif # CMWIFI
