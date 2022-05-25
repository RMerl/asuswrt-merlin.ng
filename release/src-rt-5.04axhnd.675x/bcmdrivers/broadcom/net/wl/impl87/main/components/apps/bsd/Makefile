#
# Linux bsd Makefile
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
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id: Makefile 401201 2013-05-09 03:06:42Z $
#

include $(TOP)/.config
include $(TOP)/.config.plt

CFLAGS	+= -I. -I$(TOP)/shared -I$(TOP)/bshared/include -I$(SRCBASE)/include -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(BASEDIR)/components/apps/appeventd -Wall -Werror
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))

CFLAGS += -I$(SRCBASE)/../components/math/include

#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
CFLAGS += -Wno-address-of-packed-member
LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lpthread
LDFLAGS += -L$(TOP)/bshared/src -L$(INSTALLDIR)/bshared/usr/lib -lbshared
ifeq ($(CONFIG_BCM_CEVENT),y)
LDFLAGS += -L$(TOP)/../apps/cevent_app -lceshared
CFLAGS += -DBCM_CEVENT
CFLAGS += -I$(TOP)/../apps/cevent_app          # for ce_shared.h
endif

LDFLAGS += $(EXTRA_LDFLAGS)

ifneq ($(CMWIFI),)
CFLAGS += -I$(WL_CMWIFI_DIR)/libs/wlcsm/include
LDFLAGS += -L$(INSTALL_DIR)/lib -lwlcsm
endif # CMWIFI

vpath %.c $(SRCBASE)/shared $(SRCBASE)/shared/bcmwifi/src $(WLAN_ComponentSrcDirs)

all: bsd

OBJS := bsd_main.o bsd.o bsd_engine.o  bsd_util.o bsd_comm.o bcmwifi_channels.o bsd_default.o bsd_appevent.o

ifneq (2_6,$(LINUX_VERSION))
ifeq ($(CONFIG_WBD),y)
OBJS += bsd_wbd.o
endif
endif

CFILES := $(patsubst %.o,%.c,$(OBJS))
SRC_EXIST := $(wildcard $(CFILES))

install: all
ifneq ($(wildcard bsd),)
	install -d $(INSTALLDIR)/usr/sbin/
	install bsd $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/bsd
	ln -sf bsd $(INSTALLDIR)/usr/sbin/bsdcli
endif

clean:
	rm -f *.o *.a *.so bsd

ifneq (,$(SRC_EXIST))
${OBJS}: bsd.h

bsd: bcmutils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

else
bsd:
	@true
endif
