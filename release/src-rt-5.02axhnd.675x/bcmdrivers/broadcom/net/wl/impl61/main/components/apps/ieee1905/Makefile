#
# Linux ieee1905 Makefile
#
# Copyright 2019 Broadcom
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
# $Id: Makefile 712953 2017-07-27 06:29:01Z $
#

export USEBCMUSCHED = 1

# Create a EXE to test it individually
export CREATE_IEEE1905EXE = 1

IEEE1905INSTALLDIR = $(INSTALLDIR)/ieee1905

CFLAGS  += $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS	+= -I.  -I$(TOP)/shared -I$(SRCBASE)/include -I$(WPSSOURCE_DIR)/common/include -I$(SRCBASE)/include/bcmcrypto -I$(SRCBASE)/../components/bcmcrypto/include -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/shared/proto -I$(SRCBASE)/common/include/proto -I$(WPSSOURCE_DIR)/brcm_apps/include ${WLAN_StdIncPathA} -Wall -Werror -s -O2 -fPIC
CFLAGS  += -DWIRELESS -DSUPPORT_BRIDGE_DEDICATED_PORT -DSUPPORT_FBCTL
CFLAGS  += -DSUPPORT_IEEE1905_AUTO_WDS -DSUPPORT_FCCTL
ifeq ($(USEBCMUSCHED),1)
CFLAGS  += -DUSEBCMUSCHED
endif #USEBCMUSCHED

# CFLAGS  += -DSUPPORT_IEEE1905_FM -DWANDEV_SUPPORT -DIEEE1905_KERNEL_MODULE_PB_SUPPORT -DSUPPORT_ETHSWCTL

IEEE1905_LIB	:= libieee1905.so
IEEE1905_LIB_SRCS1	:= $(shell find . -name "ieee*.c")
IEEE1905_LIB_SRCS1_TMP := $(IEEE1905_LIB_SRCS1)
IEEE1905_LIB_SRCS1 = $(filter-out ./ieee1905_main.c, $(IEEE1905_LIB_SRCS1_TMP))
IEEE1905_LIB_OBJS1	:= $(sort $(IEEE1905_LIB_SRCS1:.c=.o))

ifeq ($(CREATE_IEEE1905EXE), 1)
IEEE1905_EXE	:= ieee1905
IEEE1905_EXE_SRCS1	:= $(shell find . -name "ieee*.c")
IEEE1905_EXE_OBJS1	:= $(sort $(IEEE1905_EXE_SRCS1:.c=.o))
endif #CREATE_IEEE1905EXE

EXE2	:= i5ctl
LN2	:= i5
SRCS2	:= $(shell find . -name "i5ctl*.c")
OBJS2	:= $(SRCS2:.c=.o)

EXE3	:= ncap
SRCS3	:= $(shell find . -name "i5ncap*.c")
OBJS3	:= $(SRCS3:.c=.o)

LIB1	:= libi5api.so
LIBOBJS1:= i5api.o

LIBS = $(LIB1) $(EXTRA_LDFLAGS)
LIBS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lpthread
LIBS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LIBS += -L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/libbcmcrypto/usr/lib -lbcmcrypto

ifeq ($(CREATE_IEEE1905EXE), 1)
all: $(LIB1) $(IEEE1905_EXE) $(IEEE1905_LIB) $(EXE2) $(EXE3)
else
all: $(LIB1) $(IEEE1905_LIB) $(EXE2) $(EXE3)
endif

$(LIB1): $(LIBOBJS1)
	$(CC) -shared -o $@ $?

$(IEEE1905_LIB): $(IEEE1905_LIB_OBJS1) $(LIB1)
	$(CC) -shared -o $@ $(IEEE1905_LIB_OBJS1) $(LIBS)

$(IEEE1905_EXE): $(IEEE1905_EXE_OBJS1) $(LIB1)
	$(CC) -o $@ $(IEEE1905_EXE_OBJS1) $(LIBS)

$(EXE2): $(OBJS2) $(LIB1)
	$(CC) -o $@ $(OBJS2) $(LIBS)

$(EXE3): $(OBJS3) $(LIB1)
	$(CC) -o $@ $(OBJS3) $(LIBS)

install: all
	install -d $(IEEE1905INSTALLDIR)/usr/sbin/
	install -d $(IEEE1905INSTALLDIR)/usr/lib/
	install $(IEEE1905_LIB) $(IEEE1905INSTALLDIR)/usr/lib/
	install $(EXE2) $(IEEE1905INSTALLDIR)/usr/sbin/
	install $(EXE3) $(IEEE1905INSTALLDIR)/usr/sbin/
	install $(LIB1) $(IEEE1905INSTALLDIR)/usr/lib/
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/lib/$(IEEE1905_LIB)
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/sbin/$(EXE2)
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/sbin/$(EXE3)
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/lib/$(LIB1)
ifeq ($(CREATE_IEEE1905EXE), 1)
	install $(IEEE1905_EXE) $(IEEE1905INSTALLDIR)/usr/sbin/
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/sbin/$(IEEE1905_EXE)
endif #CREATE_IEEE1905EXE

clean:
	rm -f *.o *.a *.so $(IEEE1905_LIB) $(IEEE1905_EXE) $(EXE2) $(EXE3)
