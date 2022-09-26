#***********************************************************************
# 
# Makefile
#
#***********************************************************************
include $(LINUXDIR)/.config
include ../common.mak
include $(SRCBASE)/.config
include ../.config

CFLAGS += -I$(TOP)/libev-4.33 -I$(TOP)/libnfnetlink-1.0.1/include 
CFLAGS += -I$(TOP)/libmnl-1.0.4/include -I$(TOP)/libnfnetlink-1.0.1/include 
CFLAGS += -I$(TOP)/libnetfilter_conntrack-1.0.7/include -I$(TOP)/libnetfilter_conntrack-1.0.7/include/libnetfilter_conntrack
CFLAGS += -I$(TOP)/sqlite -DSQL -I$(TOP)/json-c
CFLAGS += -I$(TOP)/shared

LDFLAGS += -L$(TOP)/libev-4.33/.libs -lev 
LDFLAGS += -L$(TOP)/libnfnetlink-1.0.1/src/.libs -lnfnetlink 
LDFLAGS += -L$(TOP)/libmnl-1.0.4/src/.libs -lmnl 
LDFLAGS += -L$(TOP)/libnetfilter_conntrack-1.0.7/src/.libs -lnetfilter_conntrack
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -lm -lpthread -ldl -lnvram
LDFLAGS += -L$(TOP)/shared -lshared
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif

ifeq ($(RTCONFIG_LIB_CODB),y)
CFLAGS += -DCODB -I$(TOP)/libcodb
LDFLAGS += -L$(TOP)/libcodb -lcodb
endif

ifeq ($(RTCONFIG_EXT_RTL8370MB),y)
CFLAGS += -I$(LINUXDIR)/drivers/char/rtl8370mb
endif

ifeq ($(RTCONFIG_EXT_RTL8365MB),y)
CFLAGS += -I$(LINUXDIR)/drivers/char/rtl8365mb
endif

ifeq ($(RTCONFIG_EXT_BCM53134),y)
CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/bcm963xx/bcm
endif

#if CONFIG_ET is set to 'y' or 'm', then defined it
ifneq (,$(filter $(CONFIG_ET),y m))
CFLAGS += -DCONFIG_ET
CFLAGS += -I$(SRCBASE)/et/sys
endif

OBJS = log.o nfsw.o nfct.o nfarp.o nfjs.o nfsql.o nfcm.o

ifeq ($(QCA), y)
  OBJS += nffdb.o nfssdk.o 
  CFLAGS += -DQCA -DISISC # -DUMAC_SUPPORT_CFG80211=1 -DUMAC_SUPPORT_WEXT=1 -DATH_SUPPORT_EXT_STAT=1 -DATH_EXTRA_RATE_INFO_STA=0

  CFLAGS += -I$(SRCBASE)/$(PLATFORM_ROUTER)/qca-ssdk-shell/source/include
  CFLAGS += -I$(SRCBASE)/$(PLATFORM_ROUTER)/qca-ssdk-shell/source/include/api
  CFLAGS += -I$(SRCBASE)/$(PLATFORM_ROUTER)/qca-ssdk-shell/source/include/fal
  CFLAGS += -I$(SRCBASE)/$(PLATFORM_ROUTER)/qca-ssdk-shell/source/include/shell
  CFLAGS += -I$(SRCBASE)/$(PLATFORM_ROUTER)/qca-ssdk-shell/source/include/init
  CFLAGS += -I$(SRCBASE)/$(PLATFORM_ROUTER)/qca-ssdk-shell/source/include/common
  CFLAGS += -I$(SRCBASE)/$(PLATFORM_ROUTER)/qca-ssdk-shell/source/include/sal/os
  CFLAGS += -I$(SRCBASE)/$(PLATFORM_ROUTER)/qca-ssdk-shell/source/include/sal/os/linux_user
  CFLAGS += -I$(SRCBASE)/$(PLATFORM_ROUTER)/qca-wifi/source/os/linux/tools

  LDFLAGS += -L$(SRCBASE)/$(PLATFORM_ROUTER)/qca-ssdk-shell/source/build/bin -l:ssdk_us_km.a
#  LDFLAGS += -L$(SRCBASE)/$(PLATFORM_ROUTER)/qca-wifi/source/os/linux/tools -lqca_tools
#  LDFLAGS += -L$(SRCBASE)/$(PLATFORM_ROUTER)/qca-cfg80211/source/qca_nl80211_lib -lqca_nl80211_wrapper
#  LDFLAGS += -L$(SRCBASE)/$(PLATFORM_ROUTER)/libnl/source/lib/.libs -lnl-3 -lnl-genl-3

  CFLAGS += -I$(TOP)/shared/sysdeps/qca
  CFlAGS += -I$(TOP)/wireless_tools
  CFLAGS += -I$(TOP)/kernel_header/include -I$(SRCBASE)/include
  LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
else ifeq ($(HND), y)
  OBJS += nfmc.o
  CFLAGS += -DHND -I$(HND_SRC)/userspace/private/include 
  CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/bcm963xx/bcm
  CFLAGS += -I$(SRCBASE)/wl/sys
  LDFLAGS += -lethswctl
  ifeq ($(AX), y)
    CFLAGS += -I$(SRCBASE)/../components/proto/include
    CFLAGS += -I$(SRCBASE)/../components/wlioctl/include
    CFLAGS += -I$(SRCBASE)/wl 
    CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
    CFLAGS += -I$(SRCBASE)/common/include

    LDFLAGS += -L$(SRCBASE)/router-sysdep/ethswctl_lib
    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) 
    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
    LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
  else
    CFLAGS += -I$(HND_SRC)/bcmdrivers/opensource/include/bcm963xx
	CFLAGS += -I$(SRCBASE)/include
    CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
    CFLAGS += -I$(SRCBASE)/common/include

    LDFLAGS += -L$(SRCBASE)/router/ethswctl_lib
    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) 
    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
    LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
  endif
else
  OBJS += nfmc.o
  CFLAGS += -I$(SRCBASE)/wl/sys
  CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include/
  CFLAGS += -I$(SRCBASE)/common/include
  CFLAGS += -I$(SRCBASE)/include

  LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) 
  LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
endif

ifeq ($(RTCONFIG_CFGSYNC), y)
  OBJS += nfcfg.o
  CFLAGS += -DAIMESH
endif

#CFLAGS += -DNFCMDBG
#CFLAGS += -D_MACDBG_

all: nfcm

.c.o:
	$(CC) -c $*.c $(CFLAGS)

nfcm: $(OBJS)
#	@echo LDFLAGS=$(LDFLAGS)
#	@echo "=============================";
#	@echo TOP=$(TOP)
#	@echo TOP_PLATFORM=$(TOP_PLATFORM)
#	@echo SRCBASE=$(SRCBASE)
#	@echo HND_SRC=$(HND_SRC)
#	@echo BCMEX=${BCMEX}
#	@echo EX7=$(EX7)
#	@echo HND=$(HND)
#	@echo AX=$(AX)
#	@echo QCA=$(QCA)
#	@echo LINUXDIR=$(LINUXDIR)
#	@echo "=============================";
	$(CC) -o nfcm $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
#	@echo OBJS=$(OBJS)
	@echo LINUXDIR=$(LINUXDIR)
	-rm -rf nfcm stamp-h1 $(OBJS)

