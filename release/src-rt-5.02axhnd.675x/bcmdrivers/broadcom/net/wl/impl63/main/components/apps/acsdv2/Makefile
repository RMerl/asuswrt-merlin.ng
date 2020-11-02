#
# Linux acsdv2 Makefile
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
# $Id: Makefile 771599 2019-02-01 05:32:37Z $
#

WLAN_ComponentsInUse += bcmwifi math
include ../../../src/makefiles/WLAN_Common.mk
export SRCBASE := $(WLAN_SrcBaseA)
export BASEDIR := $(WLAN_TreeBaseA)
export TOP = ../../router
include $(SRCBASE)/platform.mak
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall -Werror
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include $(SRCBASE)/../components/math/include)))
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2 -fno-strict-aliasing
CFLAGS  += -DWL11AC_80P80 -DWL11AC_160
CFLAGS  += -DZDFS_2G

ifneq ($(CMWIFI),)
LDFLAGS        += -L$(INSTALL_DIR)/lib -lwlcsm
endif

CFLAGS  += $(EXTRACFLAGS)
LDFLAGS += -L$(TOP_PLATFORM)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lpthread
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm
LDFLAGS += $(EXTRA_LDFLAGS)

CFLAGS += -I../../src/shared/bcmwifi/include
CFLAGS += -I../../components/math/include
CFLAGS += -I../../wlioctl/include

vpath %.c $(SRCBASE)/shared
vpath %.c $(SRCBASE)/shared/bcmwifi/src $(SRCBASE)/../components/math/src
vpath %.c $(SRCBASE)/../components/wlioctl/include $(SRCBASE)/include

all: acsd2 acs_cli2

OBJS = bcmwifi_channels.o dcs.o acs_dfs.o acs_channels.o acs_cfg.o acs_scan.o acs_bgdfs.o acs_iov.o acs.o acsd_utils.o acsd_main.o acsd_cmd.o acsd_chanim.o acs_dfsr.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install acsd2 $(INSTALLDIR)/usr/sbin/
	install acs_cli2 $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/acsd2
	$(STRIP) $(INSTALLDIR)/usr/sbin/acs_cli2

clean:
	rm -f *.o *.a *.so acsd2 acs_cli2

${OBJS}: acsd.h acsd_svr.h acs_dfsr.h

acsd2: bcmutils.o bcmxtlv.o bcm_app_utils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

acs_cli2: acsd_cli.o acsd_utils.o bcmwifi_channels.o
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
