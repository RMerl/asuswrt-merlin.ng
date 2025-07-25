#
# Router dhd_monitor Makefile
#
# Copyright (C) 2024, Broadcom. All Rights Reserved.
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
#

include $(TOP)/.config

CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(SRCBASE)/../components/shared -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/common/include -I$(SRCBASE)/shared/bcmwifi/include/

ifeq ($(strip $(BCA_HNDROUTER)),)
CFLAGS += -I$(TOP)/busybox-1.x
endif

CFLAGS += -DDM_NO_NVRAM_RESTART
CFLAGS += -DCHECK_DUMPS_WHEN_INIT

#CFLAGS += -DCRASH_HISTORY_FILE_LOGGING

# LDFLAGS += -L$(TOP)/nvram -L$(PLATFORMDIR)/install/nvram/usr/lib -lnvram
# LDFLAGS += -L$(TOP)/shared -L$(PLATFORMDIR)/install/shared/usr/lib -lshared -lpthread
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm
LDFLAGS += -L$(TOP_PLATFORM)/nvram -L$(PLATFORMDIR)/install/nvram/usr/lib -lnvram -lpthread
LDFLAGS += -L$(TOP)/shared -L$(PLATFORMDIR)/install/shared/usr/lib -lshared
LDFLAGS += $(EXTRA_LDFLAGS)

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-BE82M GS-BE18000 GS-BE12000 GS7_PRO GT7))
LDFLAGS += -L$(TOP)/libhostapi -lhostapi
endif

OBJS := dhd_monitor.o

all: debug_monitor

clean:
	rm -f *.o dhd_monitor .*.depend

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install debug_monitor $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/debug_monitor

ifneq ($(PREBUILT_BCMBIN),1)
debug_monitor: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
else
debug_monitor:
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
endif

%.o: .%.depend
.%.depend: %.c
	$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
