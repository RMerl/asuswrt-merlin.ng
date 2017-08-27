#
# Makefile for the Broadcom EMF Layer
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
# $Id: Makefile 637881 2016-05-16 05:04:57Z $
#

# Choose BASE_DIR based on where router resides - src or components
EMF_BASE_DIR := $(firstword $(wildcard ../hnd_shared/../../../components/router/emf$(BCMEX7)	\
                                       ../hnd_shared/../../../src/router/emf$(BCMEX7)))
EMFDIR      := ../emf$(BCMEX7)
EMFSRCDIR  := $(EMFDIR)/emf
EMFCSRCDIR := ../hnd_shared/../emf/emf

SRCBASE_OFFSET := ../hnd_shared/../../../main/src
EMFFILES_SRC   := $(SRCBASE_OFFSET)/shared
SRCBASE        := $(src)/$(SRCBASE_OFFSET)
SRCBASE_ROUTER := $(SRCBASE)/router

ifeq ($(PREBUILT_EXTRAMOD),1)
REBUILD_EMF_MODULE=0
else
REBUILD_EMF_MODULE=$(shell if [ -d "$(src)/$(EMFSRCDIR)" -a "$(REUSE_PREBUILT_EMF)" != "1" ]; then echo 1; else echo 0; fi)
endif

ifeq ($(REBUILD_EMF_MODULE),1)

EMF_OBJS := $(EMFCSRCDIR)/emfc.o        $(EMFSRCDIR)/emf_linux.o

ifeq ($(CONFIG_BUZZZ_FUNC),y)
subdir-ccflags-y += -O0 -fno-inline -finstrument-functions
endif # CONFIG_BUZZZ_FUNC

# Search for sources under src/router/emf/emf
ifneq ($(wildcard $(src)/$(EMFSRCDIR)),)
    EXTRA_CFLAGS += -I$(SRCBASE_ROUTER)/emf$(BCMEX7)/emf  -I$(SRCBASE_ROUTER)/emf$(BCMEX7)/igs
    EXTRA_CFLAGS += -I$(SRCBASE)/include/emf/emf -I$(SRCBASE)/include/emf/igs
    EXTRA_CFLAGS += -I$(SRCBASE)/include         -I$(SRCBASE)/common/include
    # BCA build identifier
    EXTRA_CFLAGS += -DBCMDRIVER -DBCA_HNDROUTER
    emf-objs := $(EMF_OBJS)
endif

obj-$(CONFIG_BCM_WLAN) := emf.o

else # EMFSRCDIR doesn't exist
    # Otherwise, assume prebuilt object module(s) in src/emf/linux directory
    prebuilt := emf.o
    emf-objs := $(SRCBASE_OFFSET)/emf/linux/prebuilt/$(prebuilt)
    obj-$(CONFIG_BCM_WLAN) := emf.o
endif

clean-files += $(EMFCSRCDIR)/*.o $(EMFSRCDIR)/*.o $(EMFCSRCDIR)/.*.o.cmd $(EMFSRCDIR)/.*.o.cmd
