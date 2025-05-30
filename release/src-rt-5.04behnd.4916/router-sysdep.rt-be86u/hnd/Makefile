#
# Makefile for the Broadcom HND Layer
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
# $Id: Makefile 692718 2017-03-29 03:25:07Z $
#

HND_DIR        :=  ../hnd
# router-sysdep is symbolic link,  router-sysdep/hnd/../../ will jump to src-rt-5.02.axhnd.675x/
SRCBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src
HNDFILES_SRC   := $(SRCBASE_OFFSET)/shared
SRCBASE        := $(src)/$(SRCBASE_OFFSET)/
SRCBASE_SHARED := $(SRCBASE)/shared/bcmwifi
WLSRC_BASE     := $(src)/$(SRCBASE_OFFSET)/
WLSHARED       := wlconfig_lx_shared
WLCFGDIR       := $(WLSRC_BASE)/wl/config
WLFILES_OFFSET := $(SRCBASE_OFFSET)/../
COMMON_SHARED_DIR := ../../../../../shared/impl1
COMPONENTSBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/components
COMPONENTSBASE := $(src)/$(COMPONENTSBASE_OFFSET)/
SRCBASE_ROUTER := $(COMPONENTSBASE)/router

ifneq ($(BUILD_HND_MFG),)
export BUILD_MFG := 1
export WLTEST := 1
endif

ifneq ($(wildcard $(WLCFGDIR)),)
include $(WLCFGDIR)/$(WLSHARED)
include $(WLCFGDIR)/wl.mk
endif

include $(SRCBASE)/.config
MODEL = $(subst -,,$(BUILD_NAME))
EXTRA_CFLAGS += -D$(MODEL)

REBUILD_HND_MODULE=$(shell if [ -d "$(src)/$(HND_DIR)" -a "$(REUSE_PREBUILT_HND)" != "1" ]; then echo 1; else echo 0; fi)

ifneq ($(wildcard $(src)/$(HND_DIR)/prebuilt/hnd.o),)
REBUILD_HND_MODULE=0
endif

ifeq ($(REBUILD_HND_MODULE),1)
#Linux-4.1.0: Include the shared object files required for Wl, EMF and IGS modules
hnd-objs := $(HND_DIR)/hnd_c.o shared_ksyms.o
hnd-objs += $(addprefix $(WLFILES_OFFSET), $(sort $(WLFILES_SRC:.c=.o)))
HND_OBJS := $(addprefix $(src)/$(WLFILES_OFFSET), $(sort $(WLFILES_SRC:.c=.o)))

HND_OBJS += $(src)/$(HNDFILES_SRC)/nvram_ext.o
HND_OBJS += $(COMPONENTSBASE)/math/src/bcm_math.o

#For HNDLIB common files move
EXTRA_CFLAGS += -DWL11BE -DWL11AX -DWL11AC

#ifdef CMWIFI
ifneq ($(CMWIFI),)
KBUILD_EXTRA_SYMBOLS = $(WL_CMWIFI_DIR)/mods/wlcsm_ext/Module.symvers
ifneq ($(CMWIFI_EROUTER),)
    EXTRA_CFLAGS += -I$(WL_CMWIFI_DIR)/mods/wlerouter/include
    EXTRA_CFLAGS += -I$(LINUXDIR)/include/linux
    EXTRA_CFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/fpm
    EXTRA_CFLAGS += -DBCMEXTNVM
    hnd-objs += ../../../../cmwifi/mods/wlerouter/src/wlan_bcg_cm_linux.o
    hnd-objs += ../../../../cmwifi/mods/shared/cmwifi_srom.o
endif
endif # CMWIFI
#endif

ifneq ($(CM_BUILDROOT),)
EXTRA_CFLAGS += -DCM_BUILDROOT
endif

# Search for sources under src/router/hnd/
ifneq ($(wildcard $(src)/$(HND_DIR)),)
    EXTRA_CFLAGS += -I$(SRCBASE_SHARED)/include
    EXTRA_CFLAGS += -I$(SRCBASE)/include         -I$(SRCBASE)/common/include
    EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/bcm_knvram/impl1/include
    EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/include/$(BRCM_BOARD)
    EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/include/$(BRCM_BOARD)
    # BCA build identifier
    EXTRA_CFLAGS += $(WLFLAGS) -DBCMDRIVER
    hnd-objs += $(subst $(src)/,,$(HND_OBJS))
    EXTRA_CFLAGS += -Dlinux -DBCA_HNDROUTER -DBCM_ROUTER
ifneq ($(strip $(BUILD_HND_NIC)),)
    # BCA_HNDNIC to be used to differentiate between FD and NIC mode operation
    # on BCA_HNDROUTER platforms
    EXTRA_CFLAGS += -DBCA_HNDNIC
endif
    EXTRA_CFLAGS += -I$(COMMON_SHARED_DIR)
ifneq (,$(filter $(MODEL),RTAX58U TUFAX3000 RTAX82U RTAX82_XD6 RTAX55 RTAX1800))
    EXTRA_CFLAGS += -g -DBCMDBG -DWLMSG_ASSOC
endif

    # wl shared directory
    ifeq ($(strip $(USE_WLAN_SHARED)), 1)
         EXTRA_CFLAGS += -I$(WLSRC_BASE)/../../../shared/impl1
    endif

    ifneq ($(strip $(CONFIG_BCM_WIFI_FORWARDING_DRV)),)
        WLWFD := 1
    endif
    ifneq ($(strip $(CONFIG_BCM_ARCHER_WLAN)),)
        # Archer WLAN (implements WiFi Forwarding Driver)
        WLWFD := 1
        EXTRA_CFLAGS += -DBCM_AWL
    endif

    ifeq ($(strip $(WLWFD)), 1)
        EXTRA_CFLAGS += -DBCM_WFD
        ifneq ($(BUILD_HND_EAP_AP1),y)
            EXTRA_CFLAGS += -DPKTC -DPKTC_TBL
        endif
        EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
    endif

    ifneq ($(strip $(CONFIG_BLOG)),)
        EXTRA_CFLAGS += -DBCM_BLOG
        EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
    endif

    ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
        EXTRA_CFLAGS += -DBCM_NBUFF -DBCM_NBUFF_PKT
        EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
        EXTRA_CFLAGS += $(INC_RDP_FLAGS)
        HND_OBJS += $(src)/$(SRCBASE_OFFSET)/../../../shared/impl1/wl_nbuff.o
    endif

    # packet chaining 16-bit chain index support
    ifneq ($(strip $(CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT)),)
        EXTRA_CFLAGS += -DCONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
    endif

    ifeq ($(CMWIFI),)
        EXTRA_CFLAGS += -DBULK_PKTLIST
    endif

    ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
        EXTRA_CFLAGS += -DBCM_PKTFWD
    endif

    ifneq ($(strip $(CONFIG_BCM_WLAN_64BITPHYSADDR)),)
        EXTRA_CFLAGS += -DBCMDMA64OSL
    endif

    EXTRA_CFLAGS += -DSTS_FIFO_RXEN

    ifneq ($(strip $(CONFIG_BCM_BPM_BULK_FREE)),)
        EXTRA_CFLAGS += -DBPM_BULK_FREE
        EXTRA_CFLAGS += -DBCM_SKB_FREE_OFFLOAD
    endif

    ifeq ($(CONFIG_BCM_HND_EAP), y)
        EXTRA_CFLAGS += -DBCM_SKB_FREE_OFFLOAD
    endif

    hnd-objs += $(subst $(src)/,,$(HND_OBJS))

    WLAN_ComponentsInUse := math
    WLAN_ExposePrivateHeaders := math

    include $(WLSRC_BASE)/makefiles/WLAN_Common.mk

    EXTRA_CFLAGS += $(WLAN_ComponentIncPathA) $(WLAN_IncPathA)
endif

-include $(SRCBASE_ROUTER)/hnd/Makefile.shared

obj-$(CONFIG_BCM_WLAN) := hnd.o

$(src)/shared_ksyms.c: $(src)/shared_ksyms.sh $(HND_OBJS)
	sh -e $< $(HND_OBJS) > $@

else # HND_DIR doesn't exist

    # Otherwise, assume prebuilt object module(s) in src/hnd/linux directory
    prebuilt := hnd.o
    hnd-objs := $(SRCBASE_OFFSET)/router-sysdep/hnd/prebuilt/$(prebuilt)
    obj-$(CONFIG_BCM_WLAN) := hnd.o
endif

clean-files += $(HND_DIR)/*.o $(HNDFILES_SRC)/*.o $(HND_DIR)/shared_ksyms.c
