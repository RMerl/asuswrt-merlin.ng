#
# Broadcom Linux Router Makefile
#
# Copyright (C) 2022, Broadcom. All Rights Reserved.
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
# $Id: Makefile 790796 2020-09-03 06:16:46Z $
#

#Linux Makefile

include $(TOP)/.config

INCLUDE += -I. -I.. -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/../components/shared
INCLUDE += -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include
INCLUDE += -I$(SRCBASE)/common/include -I$(SRCBASE)/shared -I$(TOP)/shared -I$(SRCBASE)/shared/bcmwifi/include

CFLAGS += $(INCLUDE)
CFLAGS += $(WLAN_ComponentIncPathA)
CFLAGS += -Wall
CFLAGS += -DBCM_CEVENT

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
LIBCESHARED_OBJS = ce_shared.o

OBJS = $(CEVENTD_OBJS) $(CEVENTC_OBJS)

${OBJS}: cevent_app_common.h

EXES = ceventd ceventc
LIBS = libceshared.so
ALL = $(EXES) $(LIBS)

INST = $(addsuffix -install,$(ALL))

.PHONY: all clean install $(INST)

all: $(ALL)

ca_be_str.c:
	$(SHELL) ./gen_be_str.sh \
	"`echo '#include <bcmevent.h>' | $(CC) -M $(CFLAGS) -xc - | \
	grep bcmevent.h | head -1`" $@

ifneq ($(wildcard ./prebuilt/ceventd),)
ceventd:
	cp ./prebuilt/ceventd ceventd
else
ceventd: $(CEVENTD_OBJS) ceventd.h
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
endif

ifneq ($(wildcard ./prebuilt/ceventc),)
ceventc:
	cp ./prebuilt/ceventc ceventc
else
ceventc: $(CEVENTC_OBJS) ceventc.h
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
endif

ifneq ($(wildcard ./prebuilt/libceshared.so),)
libceshared.so:
	cp ./prebuilt/libceshared.so libceshared.so
else
$(LIBCESHARED_OBJS): ce_shared.h
	$(CC) -fPIC $(CFLAGS) -c $(@:.o=.c) -o $@

libceshared.so: $(LIBCESHARED_OBJS)
	$(CC) -shared -o $@ $^
endif

ceventd-install: ceventd
	install -d $(INSTALLDIR)/bin
	install ceventd $(INSTALLDIR)/bin
	$(STRIP) $(INSTALLDIR)/bin/ceventd

ceventc-install: ceventc
	install -d $(INSTALLDIR)/bin
	install ceventc $(INSTALLDIR)/bin
	$(STRIP) $(INSTALLDIR)/bin/ceventc

libceshared.so-install: libceshared.so
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 libceshared.so $(INSTALLDIR)/usr/lib
	$(STRIP) $(INSTALLDIR)/usr/lib/libceshared.so

install: $(INST)

clean:
	rm -f *.o $(ALL) ca_be_str.c
