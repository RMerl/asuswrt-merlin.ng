#
# 802.1x Authenticator (Network Access Server) Embedded Linux Build Makefile
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
# $Id: Makefile 768736 2018-10-23 21:53:02Z $
#

include ../../.config

CFLAGS	+= -I.. -I$(SRCBASE_SYS)/include -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE_SYS)/../components/shared -I$(SRCBASE_SYS)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/math/include -I$(SRCBASE_SYS)/common/include -I$(SRCBASE_ROUTER)/shared -Wall
CFLAGS	+= $(WLAN_ComponentIncPathA)
CFLAGS += -I$(SRCBASE)/../components/bcmcrypto/include -I$(SRCBASE)/shared/bcmwifi/include -I$(SRCBASE)/../components/router/shared
CFLAGS += -I$(SRCBASE)/common/include

#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s
CFLAGS  += -DNAS_WKSP_BUILD_NAS_AUTH
CFLAGS  += -DNAS_WKSP_BUILD_NAS_SUPPL
#CFLAGS  += -DNAS_GTK_PER_STA
CFLAGS  += -DNAS_RADIUS
CFLAGS  += -DNAS_WKSP_ON_DEMAND
ifeq ($(RTCONFIG_IPV6),y)
CFLAGS += -DNAS_IPV6
endif

CFLAGS	+= -DBCMSUPPL

# trunk uses bcmcrypto component
CFLAGS += -DBCMCRYPTO_COMPONENT

# EAPD include path
CFLAGS += -I$(TOP_PLATFORM)/eapd${BCMEX}$(EX7)

# NAS FBT include path
CFLAGS += -I$(SRCBASE_ROUTER)/nas${BCMEX}$(EX7)/nas_fbt

LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -L$(INSTALLDIR)/nvram/usr/lib -lnvram ${EXTRA_NV_LDFLAGS}
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lpthread
LDFLAGS += -L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/libbcmcrypto/usr/lib -lbcmcrypto
LDFLAGS += -lgcc_s

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

vpath %.c $(TOP_PLATFORM)/nas${BCMEX}$(EX7) $(TOP_PLATFORM)/nas${BCMEX}$(EX7)/nas_fbt $(SRCBASE)/shared $(SRCBASE)/router/shared $(SRCBASE)/../components/math/src

# build target
NASOBJS = mppe.o nas_wpa.o nas.o nas_linux.o nas_wl.o nas_wksp.o
NASOBJS += nas_radius.o nas_wksp_radius.o
ifeq ($(RTCONFIG_FBT),y)
NASOBJS += wpa_common.o wpa_auth_ft.o eloop.o l2_packet.o
endif
UTLOBJS = bcmwpa.o bcmutils.o bcm_math.o

ifeq ($(wildcard ../*.c), )
all:
	-cp ./prebuilt/nas .
else
all: nas
endif

nas: $(NASOBJS) $(UTLOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# install to target
install: all
	install -D nas $(INSTALLDIR)/usr/sbin/nas
	$(STRIP) $(INSTALLDIR)/usr/sbin/nas

# clean up mess
clean:
	rm -f *.o *~ nas
