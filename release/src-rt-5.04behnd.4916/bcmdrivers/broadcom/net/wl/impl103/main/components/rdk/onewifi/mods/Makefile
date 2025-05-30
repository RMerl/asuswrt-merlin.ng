#
# The rdk_nlhal Loadable Kernel Module Makefile
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
# $Id: $
#

LKM_NAME := rdk_nlhal
obj-m := $(LKM_NAME).o

#when build from source, it will go to this directory
#else it will build directly from root directory

SRCBASE = $(WLSRCBASE)

LINUXDIR_INC_BASE = $(LINUXDIR)/arch/$(ARCH)
#WLCFGDIR=${SRCBASE}/wl/config
#include $(WLCFGDIR)/wlconfig_lx_cmwifi_apsta
#include $(WLCFGDIR)/wl.mk

LKM_BASE := .

ifneq ($(CMWIFI),)
KBUILD_CFLAGS += -I$(WLSRCBASE)/include
KBUILD_CFLAGS += -I$(WLSRCBASE)/shared/bcmwifi/include
KBUILD_CFLAGS += -DBCMDRIVER -Dlinux
endif

# Needed by dhd.h
EXTRA_CFLAGS += -DLINUX

# Needed by wl_cfg80211.h, etc., header files
EXTRA_CFLAGS += -DBCMDONGLEHOST -DWLP2P -DWL_VENDOR_EXT_SUPPORT

EXTRA_CFLAGS    += -I$(WLSRCBASE)/../components/rdk/onewifi/mods/include
EXTRA_CFLAGS    += -I$(WLSRCBASE)/../components/proto/include
EXTRA_CFLAGS    += -I$(WLSRCBASE)/../components/wlioctl/include
EXTRA_CFLAGS    += -I$(WLSRCBASE)/include
EXTRA_CFLAGS    += -I$(WLSRCBASE)/../../sys/src/dhd/sys			# for dhd.h
EXTRA_CFLAGS    += -I$(WLSRCBASE)/../../sys/src/dongle/include		# for dngl_stats.h
EXTRA_CFLAGS    += -I$(WLSRCBASE)/../../sys/src/wl/sys

#ifneq ($(RDKB),)
#    -include $(LINUXBUILDDIR)/.config
#else
#    -include $(LINUXDIR)/.config
#endif # RDKB

ifneq ($(CMWIFI),)
	IFLAGS += -I$(LINUXDIR)/include/linux
ifeq ($(call KERNEL_GE,3.8.0),TRUE)
	IFLAGS += -I$(LINUXDIR)/include/uapi
	IFLAGS += -I$(LINUXDIR)/include/generated/uapi
	IFLAGS += -I$(LINUXDIR_INC_BASE)/include/generated/
endif # 3.8.0
endif #CMWIFI

EXTRA_CFLAGS += $(IFLAGS)

LKM_FILES := rdk_nl80211_hal.c ../../../../../src/shared/bcmwifi/src/bcmwifi_channels.c

LKM_OBJ_FILES := $(LKM_FILES:.c=.o)
LKM_BUILD_OBJS := $(addprefix $(LKM_BASE)/src/,$(LKM_OBJ_FILES))

$(LKM_NAME)-objs += $(LKM_BUILD_OBJS)
