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
# $Id: wps_common_lib.mk 682847 2017-02-03 11:07:17Z $
#
# Linux makefile
#

WLAN_ComponentsInUse := bcmwifi
include $(SRCBASE)/makefiles/WLAN_Common.mk

BLDTYPE = release
#BLDTYPE = debug

ifeq ($(BLDTYPE),debug)
export CFLAGS = -Wall -Wnested-externs -g -D_TUDEBUGTRACE
export CXXFLAGS = -Wall -Wnested-externs -g -D_TUDEBUGTRACE
else
export CFLAGS = -Wall -Wnested-externs
export CXXFLAGS = -Wall -Wnested-externs
endif

ifdef WCN_NET_SUPPORT
export CFLAGS += -DWCN_NET_SUPPORT
export CXXFLAGS += -DWCN_NET_SUPPORT
endif

ifeq ($(CC), arm-linux-gcc)
CFLAGS += -mstructure-size-boundary=8
STRIP = arm-linux-strip
endif
ifeq ($(CC), mipsel-uclibc-gcc)
STRIP = mipsel-uclibc-strip
endif
ifeq ($(CC), mipsel-linux-gcc)
STRIP = mipsel-linux-strip
endif
ifeq ($(CC), gcc)
STRIP = strip
endif

export LD = $(CC)
export LDFLAGS = -r

export INCLUDE = -I$(SRCBASE)/include -I$(SRCBASE)/common/include -I./include $(WLAN_ComponentIncPathR) $(WLAN_StdIncPathR)

# Include external openssl path
ifeq ($(EXTERNAL_OPENSSL),1)
export INCLUDE += -DEXTERNAL_OPENSSL -include wps_openssl.h -I$(EXTERNAL_OPENSSL_INC) -I$(EXTERNAL_OPENSSL_INC)/openssl
else
export INCLUDE += -I$(SRCBASE)/include/bcmcrypto
endif

LIBDIR = .

OBJS =  $(addprefix $(LIBDIR)/, shared/tutrace.o \
	shared/dev_config.o \
	shared/wps_sslist.o \
	enrollee/enr_reg_sm.o \
	shared/reg_proto_utils.o \
	shared/reg_proto_msg.o \
	shared/tlv.o \
	shared/state_machine.o \
	shared/wps_utils.o \
	shared/ie_utils.o \
	shared/buffobj.o )

# Add OpenSSL wrap API if EXTERNAL_OPENSSL defined
ifeq ($(EXTERNAL_OPENSSL),1)
OBJS += $(addprefix $(LIBDIR)/, shared/wps_openssl.o)
endif

#Be aware share library may has "-m32" issue.
ifeq ($(CC), gcc)
default: $(LIBDIR)/libwpscom.a
else
default: $(LIBDIR)/libwpscom.a $(LIBDIR)/libwpscom.so
endif

$(LIBDIR)/libwpscom.a: $(OBJS)
	$(AR) cr  $@ $^

$(LIBDIR)/libwpscom.so: $(OBJS)
	$(LD) -shared -o $@ $^

$(LIBDIR)/shared/%.o :  shared/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

$(LIBDIR)/ap/%.o :  ap/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

$(LIBDIR)/enrollee/%.o :  enrollee/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

$(LIBDIR)/registrar/%.o :  registrar/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@
