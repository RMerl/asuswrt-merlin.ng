#
# Makefile for the Broadcom IGS Layer
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
# $Id$
#
ifeq ($(strip $(USE_WLAN_SHARED)), 1)
ifneq ($(strip $(WLAN_SHARED_DIR)),)
-include $(WLAN_SHARED_DIR)/wifi_cfg_common.mk
endif
endif

# Choose BASE_DIR based on where router resides - src or components
EMF_BASE_DIR := $(firstword $(wildcard ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/components/router/emf	\
                                       ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/components/src/router/emf))

EMFDIR       := ../emf
IGSSRCDIR    := $(EMFDIR)/igs
IGSCSRCDIR   := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/components/emf/igs

SRCBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src
IGSFILES_SRC   := $(SRCBASE_OFFSET)/shared
SRCBASE        := $(src)/$(SRCBASE_OFFSET)
COMPONENTSBASE_OFFSET := ../../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/components/
COMPONENTSBASE := $(src)/$(COMPONENTSBASE_OFFSET)
SRCBASE_ROUTER := $(COMPONENTSBASE)/router-sysdep

ifeq ($(PREBUILT_EXTRAMOD),1)
REBUILD_IGS_MODULE=0
else
REBUILD_IGS_MODULE=$(shell if [ -d "$(src)/$(IGSSRCDIR)" -a "$(REUSE_PREBUILT_IGS)" != "1" ]; then echo 1; else echo 0; fi)
endif

ifneq ($(wildcard $(src)/../../router-sysdep/hnd_igs/prebuilt/igs.o),)
REBUILD_IGS_MODULE=0
endif

ifeq ($(REBUILD_IGS_MODULE),1)

IGS_OBJS := $(IGSCSRCDIR)/igsc.o $(IGSCSRCDIR)/igsc_sdb.o $(IGSSRCDIR)/igs_linux.o \
	    $(IGSCSRCDIR)/osl_linux.o

ifeq ($(BUILD_NBUFF_WLMCAST_IPV6),y)
IGS_OBJS += $(IGSCSRCDIR)/igsc_ipv6.o $(IGSCSRCDIR)/igsc_sdb_ipv6.o
endif

ifneq ($(CMWIFI_WMF_IPV6),)
EXTRA_CFLAGS += -DCMWIFI_WMF_IPV6
IGS_OBJS += $(IGSCSRCDIR)/igsc_sdb_ipv6.o
endif

# Search for sources under src/router/emf/igs
ifneq ($(wildcard $(src)/$(IGSSRCDIR)),)
    EXTRA_CFLAGS += -I$(SRCBASE_ROUTER)/emf/emf  -I$(SRCBASE_ROUTER)/emf/igs
    EXTRA_CFLAGS += -I$(SRCBASE_ROUTER)/bcmdrv/include -I$(SRCBASE_ROUTER)/bcmdrv/include/emf/igs
    EXTRA_CFLAGS += -I$(SRCBASE_ROUTER)/bcmdrv/include/emf/emf
    EXTRA_CFLAGS += -I$(SRCBASE)/include         -I$(COMPONENTSBASE)/proto/include
    # BCA build identifier
    EXTRA_CFLAGS += -DBCMDRIVER -DBCA_HNDROUTER
    igs-objs := $(IGS_OBJS)
endif

-include $(SRCBASE_ROUTER)/hnd/Makefile.shared

obj-$(CONFIG_BCM_WLAN) := igs.o

else # IGSSRCDIR doesn't exist
    # Otherwise, assume prebuilt object module(s) in src/igs/linux directory
    prebuilt := igs.o
    igs-objs := $(SRCBASE_OFFSET)/router-sysdep/hnd_igs/prebuilt/$(prebuilt)
    obj-$(CONFIG_BCM_WLAN) := igs.o
endif

clean-files += $(IGSCSRCDIR)/*.o $(IGSSRCDIR)/*.o $(IGSCSRCDIR)/.*.o.cmd $(IGSSRCDIR)/.*.o.cmd
