#
# Miscellaneous utilities Makefile
#
# Copyright (C) 2009, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile 787961 2020-06-17 07:00:35Z $
#

include $(SRCBASE)/router/.config

UTILS := epi_ttcp
CFLAGS += -s -I$(SRCBASE)/include -I$(SRCBASE)/../components/shared -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/common/include
ifeq ($(CONFIG_RTR_OPTIMIZE_SIZE),y)
CFLAGS  += -Os
else
CFLAGS  += -O2
endif

CFLAGS += -DLINUX

CFLAGS += ${WLAN_StdIncPathA}
CFLAGS += $(if $(WLAN_ComponentIncPathA),$(WLAN_ComponentIncPathA),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))

ifeq ($(CONFIG_TRAFFIC_MGMT),y)
CFLAGS += -DTRAFFIC_MGMT
endif

## Enable DMA RX overflow fix
ifeq ($(ET_INGRESS_QOS),1)
CFLAGS += -DET_INGRESS_QOS
endif

vpath %.c $(SRCBASE)/tools/misc $(WLAN_StdSrcDirsA) $(WLAN_ComponentSrcDirsA)

ifneq ($(wildcard $(SRCBASE)/et/linux/et.c),)
UTILS += et
vpath %.c $(SRCBASE)/et/linux
endif

LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram ${EXTRA_NV_LDFLAGS}
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lpthread
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
LDFLAGS += -lgcc_s

# contains e.g. WlGetDriverCfg.sh
export APPS_UTILS_DIR := $(BASEDIR)/components/apps/utils

all: $(UTILS)

clean:
	rm -f *.o *~ $(UTILS)

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install $(UTILS) $(INSTALLDIR)/usr/sbin
	$(STRIP) $(foreach file,$(UTILS),$(INSTALLDIR)/usr/sbin/$(file))
	install -m 755 $(APPS_UTILS_DIR)/WlGetDriverCfg.sh $(INSTALLDIR)/usr/sbin
	install -m 755 $(APPS_UTILS_DIR)/WlGetDriverStats.sh $(INSTALLDIR)/usr/sbin
	install -m 755 $(APPS_UTILS_DIR)/debug_loop_acs_dfs.sh $(INSTALLDIR)/usr/sbin
	install -m 755 $(APPS_UTILS_DIR)/mapgetcfg.sh $(INSTALLDIR)/usr/sbin
