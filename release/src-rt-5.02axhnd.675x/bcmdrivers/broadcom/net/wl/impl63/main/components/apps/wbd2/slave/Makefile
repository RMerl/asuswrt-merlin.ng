#
# Linux WBD Slave Makefile
#
# Copyright 2020 Broadcom
#
# This program is the proprietary software of Broadcom and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
# WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
# THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof, and to
# use this information only in connection with your use of Broadcom
# integrated circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
# OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
# SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
# IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
# IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
# ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
# OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
# NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
# $Id: Makefile 770153 2018-12-06 10:59:51Z $
#

include $(SRCBASE_ROUTER)/.config

ifneq ($(NO_BCMINTERNAL), 1)
CFLAGS	+= -DBCMINTERNAL
endif
CFLAGS	+= -DBCMDBG
CFLAGS  += -DMULTIAP

SHAREDDIR = ../shared
PREBUILTDIR = ../prebuilt
BLANKETDIR = ../blanket
IEEE1905DIR = ieee1905
IEEE1905PATH = ../../$(IEEE1905DIR)
TARGET = wbd_slave
WBDINSTALLDIR = $(INSTALLDIR)/wbd
SRCSHAREDDIR = $(SRCBASE)/shared

CFLAGS	+= -I. -I$(TOP)/shared -I$(TOP)/bshared/include -I$(SRCBASE)/../components/opensource/jsonc \
		-I$(SRCBASE)/include -I$(SRCBASE)/../components/shared \
		-I$(SRCBASE)/common/include -I$(SRCBASE)/../components/wlioctl/include	\
		-I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/math/include \
		-I$(SRCBASE)/common/include/proto -I$(SRCBASE)/../components/shared/proto

ifeq ($(CONFIG_BCM_APPEVENTD), y)
	CFLAGS += -I$(SRCBASE)/router/appeventd
	CFLAGS += -I$(SRCBASE)/../components/router/appeventd
	CFLAGS += -I$(SRCBASE)/../components/apps/appeventd
endif
CFLAGS += -I$(SHAREDDIR) -I$(IEEE1905PATH) -I$(BLANKETDIR)
CFLAGS  += $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS	+= -s -O2
LDFLAGS += -L$(SRCBASE)/../components/opensource/jsonc/installbin/lib
LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lpthread
LDFLAGS += -L$(TOP)/bshared/src -L$(INSTALLDIR)/bshared/usr/lib -lbshared
LDFLAGS += -L$(SHAREDDIR) -L$(INSTALLDIR)/wbd/usr/lib -lwbdshared
LDFLAGS += -L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/usr/bin/libbmcrypto -lbcmcrypto
LDFLAGS += -L$(IEEE1905PATH) -L$(INSTALLDIR)/$(IEEE1905DIR)/usr/lib -lieee1905
LDFLAGS += -L$(IEEE1905PATH) -L$(INSTALLDIR)/$(IEEE1905DIR)/usr/lib -li5api

ifeq ($(PLC_WBD),1)
CFLAGS	+= -I$(SRCBASE)/include/homeplugctl_drv
LDFLAGS += -L$(TOP)/homeplugctl_drv -L$(INSTALLDIR)/homeplugctl_drv/usr/lib -lhomeplugctl_drv
endif

LDFLAGS += $(EXTRA_LDFLAGS)

CFLAGS += -Wall -Werror
LDFLAGS += -ljson-c

all: $(TARGET)

SLAVE_INC = wbd_wl_utility.h wbd_blanket_utility.h wbd_slave_control.h wbd_slave_com_hdlr.h

OBJS = $(SLAVE_INC) wbd_wl_utility.o wbd_blanket_utility.o wbd_slave_control.o \
	wbd_slave_com_hdlr.o wbd_slave.o
ifeq ($(PLC_WBD),1)
OBJS +=  wbd_plc_utility.o
endif

install: all
	install $(TARGET) $(PREBUILTDIR)

clean:
	rm -f *.o *.a *.so *.bak $(TARGET) *~

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
