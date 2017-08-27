#
# Makefile for the Broadcom HND Layer
#
# Copyright (C) 2016, Broadcom. All Rights Reserved.
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
# $Id: Makefile 655682 2016-08-23 05:17:29Z $
#

HND_DIR        :=  ../hnd
SRCBASE_OFFSET := ../hnd_shared/../../../main/src
HNDFILES_SRC   := $(SRCBASE_OFFSET)/shared
SRCBASE        := $(src)/$(SRCBASE_OFFSET)
SRCBASE_SHARED := $(SRCBASE)/shared/bcmwifi
WLSRC_BASE     := $(src)$(SRCBASE_OFFSET)
WLSHARED       := wlconfig_lx_shared
WLCFGDIR       := $(WLSRC_BASE)/wl/config

ifneq ($(BUILD_HND_MFG),)
export BUILD_MFG := 1
export WLTEST := 1
endif

ifneq ($(wildcard $(WLCFGDIR)),)
include $(WLCFGDIR)/$(WLSHARED)
include $(WLCFGDIR)/wl.mk
endif

ifeq ($(PREBUILT_EXTRAMOD),1)
REBUILD_HND_MODULE=0
else
REBUILD_HND_MODULE=$(shell if [ -d "$(src)/$(HND_DIR)" -a "$(REUSE_PREBUILT_HND)" != "1" ]; then echo 1; else echo 0; fi)
endif

ifeq ($(REBUILD_HND_MODULE),1)
#Linux-4.1.0: Include the shared object files required for Wl, EMF and IGS modules
hnd-objs := $(HND_DIR)/hnd_c.o shared_ksyms.o
hnd-objs += $(addprefix $(HNDFILES_SRC)/, $(sort $(WLFILES:.c=.o)))
HND_OBJS := $(addprefix $(src)/$(HNDFILES_SRC)/, $(sort $(WLFILES:.c=.o)))
HND_OBJS += $(src)/$(HNDFILES_SRC)/nvram_ext.o

ifeq ($(CONFIG_BUZZZ_FUNC),y)
subdir-ccflags-y += -O0 -fno-inline -finstrument-functions
endif # CONFIG_BUZZZ_FUNC

# Search for sources under src/router/hnd/
ifneq ($(wildcard $(src)/$(HND_DIR)),)
    EXTRA_CFLAGS += -I$(SRCBASE_SHARED)/include
    EXTRA_CFLAGS += -I$(SRCBASE)/include         -I$(SRCBASE)/common/include
    EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/wlcsm_ext/impl1/include
    # BCA build identifier
    EXTRA_CFLAGS += $(WLFLAGS) -DBCMDRIVER
    hnd-objs += $(subst $(src)/,,$(HND_OBJS))
endif

include $(SRCBASE)/.config
MODEL = $(subst -,,$(BUILD_NAME))
EXTRA_CFLAGS += -D$(MODEL)

obj-$(CONFIG_BCM_WLAN) := hnd.o

$(src)/shared_ksyms.c: $(src)/shared_ksyms.sh $(HND_OBJS)
	sh -e $< $(HND_OBJS) > $@

else # HND_DIR doesn't exist
    # Otherwise, assume prebuilt object module(s) in src/hnd/linux directory
    prebuilt := hnd.o
    hnd-objs := $(SRCBASE_OFFSET)/hnd/linux/prebuilt/$(prebuilt)
    obj-$(CONFIG_BCM_WLAN) := hnd.o
endif

clean-files += $(HND_DIR)/*.o $(HNDFILES_SRC)/*.o $(HND_DIR)/shared_ksyms.c
