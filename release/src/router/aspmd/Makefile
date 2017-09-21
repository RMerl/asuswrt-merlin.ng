#
# aspmd Makefile
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: $
#

CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall
CFLAGS += $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS += -s -O2


LDFLAGS += -L$(TOP)/nvram${BCMEX}${EX7} -L$(INSTALLDIR)/nvram/usr/lib -lnvram ${EXTRA_NV_LDFLAGS}
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += -lgcc_s

ifeq ($(wildcard $(SRCBASE)/router/aspmd/*.c),)
all:
	-cp -f prebuilt/aspmd aspmd
else
all: aspmd
endif

OBJS = aspmd_main.o

clean:
	rm -f *.o aspmd

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install aspmd $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/aspmd

aspmd: ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
