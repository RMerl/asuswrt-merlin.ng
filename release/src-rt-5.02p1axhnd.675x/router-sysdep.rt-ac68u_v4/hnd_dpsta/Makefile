#
# Makefile for the Dualband Proxy STA
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
# $Id$
#
include ../../.config

ifeq ($(strip $(USE_WLAN_SHARED)), 1)
ifneq ($(strip $(WLAN_SHARED_DIR)),)
-include $(WLAN_SHARED_DIR)/wifi_cfg_common.mk
endif
endif

obj-${CONFIG_BCM_WLAN} := dpsta.o

DPSTA_BASE_DIR := $(firstword $(wildcard ../../../../components/router-sysdep/dpsta	\
                                         ../../../../src/router-sysdep/dpsta))
DPSTASRC := ../dpsta
DPSTAOBJ := $(DPSTASRC)/dpsta_linux.o
SYMOBJ := $(DPSTASRC)/dpsta_linux.o

SRCBASE_OFFSET := ../../../../main/src
SRCBASE        := $(src)/$(SRCBASE_OFFSET)
COMPONENTSBASE_OFFSET := ../../../components
COMPONENTSBASE := $(src)/$(COMPONENTSBASE_OFFSET)

# Search for sources under src/router/dpsta
ifneq (,$(wildcard $(src)/$(DPSTASRC)/*.c))
  EXTRA_CFLAGS += -I$(src)/$(DPSTASRC)/../../router-sysdep/dpsta
  EXTRA_CFLAGS += -I$(SRCBASE)/include -I$(SRCBASE)/common/include
  EXTRA_CFLAGS += -I$(COMPONENTSBASE)/proto/include
  EXTRA_CFLAGS += -DHAVE_NET_DEVICE_OPS
  # BCA build identifier
  EXTRA_CFLAGS += -DBCMDRIVER -DBCA_HNDROUTER
  EXTRA_CFLAGS += -Wno-date-time
  dpsta-objs := $(DPSTAOBJ) dpsta_ksyms.o
else # DPSTASRC not exist
  # Otherwise, assume prebuilt object module(s) in src/dpsta/linux directory
  prebuilt := dpsta.o
ifeq ($(HND_ROUTER_AX_6710),y)
  dpsta-objs := $(SRCBASE_OFFSET)/dpsta/linux.4908/impl$(BCM_WLIMPL)/$(prebuilt)
  ifeq (,$(wildcard $(src)/$(dpsta-objs)))
    dpsta-objs := $(DPSTASRC)/linux.4908/impl$(BCM_WLIMPL)/$(prebuilt)
  endif
else
  dpsta-objs := $(SRCBASE_OFFSET)/dpsta/linux/$(prebuilt)
  ifeq (,$(wildcard $(src)/$(dpsta-objs)))
    dpsta-objs := $(DPSTASRC)/linux/$(prebuilt)
  endif
endif
endif

PHONY += $(src)/dpsta_ksyms.c

quiet_cmd_ksyms = CALL    $<
      cmd_ksyms = $(CONFIG_SHELL) -e $< $(src)/$(SYMOBJ) > $@

$(src)/dpsta_ksyms.c: $(src)/dpsta_ksyms.sh $(src)/$(SYMOBJ) FORCE
	$(call cmd,ksyms)

clean-files += $(DPSTASRC)/*.o $(DPSTASRC)/.*.o.cmd
