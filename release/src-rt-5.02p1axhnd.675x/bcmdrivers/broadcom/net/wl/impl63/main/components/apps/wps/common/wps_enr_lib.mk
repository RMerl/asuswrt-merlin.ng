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
# $Id: wps_enr_lib.mk 768737 2018-10-23 22:07:55Z $
#
# Linux Makefile
#

WLAN_ComponentsInUse := bcmwifi
include $(SRCBASE)/makefiles/WLAN_Common.mk

BLDTYPE = release
#BLDTYPE = debug

CC = gcc
export $(CC)
export LD = $(CC)

ifeq ($(CC), arm-linux-gcc)
CFLAGS += -mstructure-size-boundary=8
#LD = arm-linux-gcc
endif

#ifeq ($(CC), mipsel-uclibc-gcc)
#LD = mipsel-uclibc-gcc
#endif // endif

ifeq ($(BLDTYPE),debug)
export CFLAGS = -Wall -Wnested-externs -g -D_TUDEBUGTRACE -DWPS_WIRELESS_ENROLLEE
export CXXFLAGS = -Wall -Wnested-externs -g -D_TUDEBUGTRACE -DWPS_WIRELESS_ENROLLEE
else
export CFLAGS = -Os -Wall -Wnested-externs -DWPS_WIRELESS_ENROLLEE
export CXXFLAGS = -Os -Wall -Wnested-externs -DWPS_WIRELESS_ENROLLEE
endif

export LDFLAGS = -r
export INCLUDE = -I$(SRCBASE)/include -I$(SRCBASE)/common/include -I./include $(WLAN_ComponentIncPathR) $(WLAN_StdIncPathR)

CRYPTDIR = ../../bcmcrypto

LIBDIR = .

ifeq ($(EXTERNAL_OPENSSL),1)
export INCLUDE += -DEXTERNAL_OPENSSL -include wps_openssl.h -I$(EXTERNAL_OPENSSL_INC) -I$(EXTERNAL_OPENSSL_INC)/openssl
else
export INCLUDE += -I$(SRCBASE)/include/bcmcrypto
CRYPTOBJS := $(addprefix $(LIBDIR)/$(notdir $(CRYPTDIR))/, aes.o rijndael-alg-fst.o \
	 dh.o bn.o sha256.o hmac_sha256.o random.o)
endif

OBJS =  $(addprefix $(LIBDIR)/, enrollee/enr_api.o \
	sta/sta_eap_sm.o \
	enrollee/enr_reg_sm.o \
	registrar/reg_sm.o )

#Be aware share library may has "-m32" issue.
ifeq ($(CC), gcc)
default: $(LIBDIR)/libwpsenr.a $(LIBDIR)/libbcmcrypto.a
else
default: $(LIBDIR)/libwpsenr.a $(LIBDIR)/libbcmcrypto.a $(LIBDIR)/libwpsenr.so
endif

$(LIBDIR)/libbcmcrypto.a: $(CRYPTOBJS)
	$(AR) cr  $@ $^

$(LIBDIR)/libwpsenr.a: $(OBJS)
	$(AR) cr  $@ $^

$(LIBDIR)/libbcmcrypto.so: $(CRYPTOBJS)
	$(LD) -shared -o $@ $^

$(LIBDIR)/libwpsenr.so: $(OBJS)
	$(LD) -shared -o $@ $^

$(LIBDIR)/bcmcrypto/%.o :  $(CRYPTDIR)/%.c
	$(CC) -c $(CFLAGS) -DAES_CBC_ONLY $(INCLUDE) $< -o $@

$(LIBDIR)/shared/%.o :  shared/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

$(LIBDIR)/sta/%.o :  sta/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

$(LIBDIR)/enrollee/%.o :  enrollee/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

$(LIBDIR)/registrar/%.o :  registrar/%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@
