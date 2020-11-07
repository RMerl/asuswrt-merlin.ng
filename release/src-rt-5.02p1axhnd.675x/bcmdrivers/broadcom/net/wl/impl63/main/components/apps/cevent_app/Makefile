#
# Broadcom Linux Router Makefile
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
#
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id: Makefile 768738 2018-10-23 22:21:02Z $
#

#Linux Makefile

include $(TOP)/.config

INCLUDE += -I. -I.. -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/../components/shared
INCLUDE += -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include
INCLUDE += -I$(SRCBASE)/common/include -I$(SRCBASE)/shared -I$(TOP)/shared -I$(SRCBASE)/shared/bcmwifi/include

CFLAGS += $(INCLUDE)
CFLAGS += $(WLAN_ComponentIncPathA)
CFLAGS += -Wall

CFLAGS  += -s

ifneq ($(LINUXDIR),)
CFLAGS += -Dlinux
endif

# trunk uses bcmcrypto component
CFLAGS += -DBCMCRYPTO_COMPONENT

LDFLAGS	+= -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram -lnvram -L$(TOP_PLATFORM)/wlcsm -lwlcsm $(EXTRA_LDFLAGS) -lpthread

vpath %.c $(SRCBASE)/../components/apps/cevent_app $(SRCBASE)/shared

CEVENTD_OBJS = cevent_app_common.o ceventd.o ceventd_utils.o ca_be_str.o
CEVENTC_OBJS = cevent_app_common.o ceventc.o ceventc_report.o

OBJS = $(CEVENTD_OBJS) $(CEVENTC_OBJS)

${OBJS}: cevent_app_common.h

EXES = ceventd ceventc

INST = $(addsuffix -install,$(EXES))

.PHONY: all clean install $(INST)

all: $(EXES)

ca_be_str.c:
	$(SHELL) ./gen_be_str.sh \
	"`echo '#include <bcmevent.h>' | $(CC) -M $(CFLAGS) -xc - | \
	grep bcmevent.h | head -1`" $@

ifneq ($(PREBUILT_BCMBIN),1)
ceventd: $(CEVENTD_OBJS) ceventd.h
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

ceventc: $(CEVENTC_OBJS) ceventc.h
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
else
ceventd:
	cp -f $(SRCBASE)/../components/apps/cevent_app/$(PRBM_DIR)/$@ ./
ceventc:
	cp -f $(SRCBASE)/../components/apps/cevent_app/$(PRBM_DIR)/$@ ./	
endif
ceventd-install: ceventd
	install -d $(INSTALLDIR)/bin
	install ceventd $(INSTALLDIR)/bin
	$(STRIP) $(INSTALLDIR)/bin/ceventd

ceventc-install: ceventc
	install -d $(INSTALLDIR)/bin
	install ceventc $(INSTALLDIR)/bin
	$(STRIP) $(INSTALLDIR)/bin/ceventc

install: $(INST)

clean:
	rm -f *.o $(EXES) ca_be_str.c

