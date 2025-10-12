# Broadcom Set Top Box(STB) AP Linux Makefile for WLCSM Kernel Module
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

WLCSM_LKM_NAME := wlcsm
obj-m +=$(WLCSM_LKM_NAME).o

#when build from source, it will got to wlcsm_ext directory
#else it will build directly from root directory

#for header file to be included
# CMWIFI start
# copied from include ${SRCBASE}/makefiles/cm_linux_modules.mk
ifneq ($(CMWIFI),)
KBUILD_CFLAGS += -DCMWIFI=1
KBUILD_CFLAGS += -DMAX_NVRAM_SPACE=$(MAX_NVRAM_SIZE)
KBUILD_CFLAGS += -I$(WLSRCBASE)/include
KBUILD_CFLAGS += -I$(WLSRCBASE)/shared/bcmwifi/include
KBUILD_CFLAGS += $(WLAN_ComponentIncPath)
KBUILD_CFLAGS += $(WLAN_StdIncPathA)
KBUILD_CFLAGS += -I$(WLSRCBASE)/common/include
KBUILD_CFLAGS += -DBCMDRIVER -Dlinux
endif

#include $(WLCFGDIR)/$(DEFAULT_CONFIG)
WLCFGDIR=${WLSRCBASE}/wl/config
#-include $(WLCFGDIR)/wlconfig_lx_wl_stb_pci_ap
EXTRA_CFLAGS += -I${M}/include
# CMWIFI end
include $(WLCFGDIR)/wl.mk

WLCSM_FILES := wlcsm_linux.c
WLCSM_FILES += wlcsm_netlink.c wlcsm_nvram.c
WLCSM_OBJ_FILES := $(WLCSM_FILES:.c=.o)
WLCSM_BUILD_OBJS := $(addprefix src/,$(WLCSM_OBJ_FILES))

obj-m += $(WLCSM_LKM_NAME).o
$(WLCSM_LKM_NAME)-objs += $(WLCSM_BUILD_OBJS)
