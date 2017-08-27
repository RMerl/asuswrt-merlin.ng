#
# Makefile for the Dualband Proxy STA
#
# Copyright (C) 2017, Broadcom. All Rights Reserved.
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
# $Id: Makefile 517800 2014-11-26 19:35:30Z $
#

obj-${CONFIG_BCM_WLAN} := dpsta.o

DPSTA_BASE_DIR := $(firstword $(wildcard ../../../../components/router/dpsta	\
                                         ../../../../src/router/dpsta))
DPSTASRC := ../dpsta
DPSTAOBJ := $(DPSTASRC)/dpsta_linux.o
SYMOBJ := $(DPSTASRC)/dpsta_linux.o

SRCBASE_OFFSET := ../../../../main/src
SRCBASE        := $(src)/$(SRCBASE_OFFSET)

# Search for sources under src/router/dpsta
ifneq (,$(wildcard $(src)/$(DPSTASRC)/*.c))
  ifeq ($(CONFIG_BUZZZ_FUNC),y)
  subdir-ccflags-y += -O0 -fno-inline -finstrument-functions
  endif # CONFIG_BUZZZ_FUNC

  EXTRA_CFLAGS += -I$(src)/$(DPSTASRC)/../../router/dpsta
  EXTRA_CFLAGS += -I$(SRCBASE)/include -I$(SRCBASE)/common/include
  # BCA build identifier
  EXTRA_CFLAGS += -DBCMDRIVER -DBCA_HNDROUTER
  dpsta-objs := $(DPSTAOBJ) dpsta_ksyms.o
else # DPSTASRC not exist
  # Otherwise, assume prebuilt object module(s) in src/dpsta/linux directory
  prebuilt := dpsta.o
  dpsta-objs := $(DPSTASRC)/linux/$(prebuilt)
endif

PHONY += $(src)/dpsta_ksyms.c

quiet_cmd_ksyms = CALL    $<
      cmd_ksyms = $(CONFIG_SHELL) -e $< $(src)/$(SYMOBJ) > $@

$(src)/dpsta_ksyms.c: $(src)/dpsta_ksyms.sh $(src)/$(SYMOBJ) FORCE
	$(call cmd,ksyms)

clean-files += $(DPSTASRC)/*.o $(DPSTASRC)/.*.o.cmd
