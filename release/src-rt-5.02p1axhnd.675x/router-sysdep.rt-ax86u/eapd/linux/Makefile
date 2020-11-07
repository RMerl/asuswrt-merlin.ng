#Linux Makefile

include ../../.config

INCLUDE += -I. -I.. -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/../components/shared -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/common/include -I$(TOP)/shared -I../../shared -I../../nas${BCMEX}$(EX7) -I../../nas${BCMEX}$(EX7)/nas_fbt
INCLUDE += -I$(SRCBASE)/shared/bcmwifi/include -I$(SRCBASE)/../components/bcmcrypto/include -I$(TOP_PLATFORM)/nas${BCMEX}$(EX7)/nas

ifneq ($(CMWIFI),)
INCLUDE += -I$(SRCBASE)/shared/bcmwifi/include
endif

CFLAGS += $(INCLUDE)
CFLAGS += $(WLAN_ComponentIncPathA)
CFLAGS += -DEAPD_WKSP_AUTO_CONFIG
CFLAGS += -I$(HND_SRC)/router-sysdep/eapd/

ifneq ($(CMWIFI),)
ifneq ($(BUILD_PASSPOINT_R2),)
CFLAGS += -DNAS_GTK_PER_STA
endif
endif

#CFLAGS += -DNAS_GTK_PER_STA
CFLAGS += -Wall -DEAPDDUMP

CFLAGS  += -s

ifneq ($(LINUXDIR),)
CFLAGS += -Dlinux
endif

# trunk uses bcmcrypto component
CFLAGS += -DBCMCRYPTO_COMPONENT

ifneq ($(CMWIFI),)
CFLAGS += -DBCM_NETXL
ifneq ($(DSLCPE_WLCSM_EXT),)
LDFLAGS+=-lwlcsm
endif
endif
LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -L$(INSTALLDIR)/nvram/usr/lib -lnvram ${EXTRA_NV_LDFLAGS}
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lpthread
LDFLAGS += $(EXTRA_LDFLAGS)

vpath %.c $(TOP_PLATFORM)/eapd${BCMEX}$(EX7) $(TOP)/shared

OBJS = eapd.o eapd_linux.o
OBJS += nas_eap.o wps_eap.o

#ifdef BCMWAPI_WAI
ifeq ($(CONFIG_WAPI),y)
OBJS += wai_eap.o
endif
#endif // endif

#ifdef BCM_CUSTOM_EVENT
ifeq ($(CONFIG_BCM_CUSTOM_EVENT),y)
OBJS +=  custom_eventd_eap.o
CFLAGS += -DBCM_CUSTOM_EVENT
endif
#endif // endif

#ifdef BCM_DCS
ifeq ($(RTCONFIG_BCMDCS),y)
OBJS += dcs_eap.o
CFLAGS += -DBCM_DCS
endif
#endif // endif

#ifdef BCM_MEVENT
ifeq ($(CONFIG_BCM_MEVENT),y)
OBJS += mevent_eap.o
CFLAGS += -DBCM_MEVENT
endif
#endif // endif

#ifdef BCM_BSD
ifeq ($(RTCONFIG_BCMBSD),y)
OBJS += bsd_eap.o
CFLAGS += -DBCM_BSD
endif
#endif // endif

#ifdef BCM_DRSDBD
ifeq ($(RTCONFIG_BCMDRSDBD),y)
OBJS += drsdbd_eap.o
CFLAGS += -DBCM_DRSDBD
endif
#endif // endif

#ifdef BCM_SSD
ifeq ($(RTCONFIG_BCMSSD),y)
OBJS += ssd_eap.o
CFLAGS += -DBCM_SSD
endif
#endif // endif

#ifdef BCM_EVENTD
ifeq ($(RTCONFIG_BCMEVENTD),y)
OBJS += eventd_eap.o
CFLAGS += -DBCM_EVENTD
endif
#endif // endif

ifneq ($(BCM_ECBD),)
OBJS += ecbd_eap.o
CFLAGS += -DBCM_ECBD
endif

#ifdef BCM_ASPMD
ifneq (2_6,$(LINUX_VERSION))
ifeq ($(CONFIG_BCMASPMD),y)
OBJS += aspm_eap.o
CFLAGS += -DBCM_ASPMD
endif
endif
#endif // endif

#ifdef CONFIG_VISUALIZATION
ifeq ($(RTCONFIG_VISUALIZATION),y)
OBJS += visdcoll_eap.o
endif
#endif // endif

#ifdef BCM_CEVENT
ifeq ($(CONFIG_BCM_CEVENT),y)
OBJS += cevent_eap.o
CFLAGS += -DBCM_CEVENT
endif
#endif // endif

OBJS += rgd_eap.o
CFLAGS += -DBCM_RGD

#ifdef BCM_WBD
ifeq ($(CONFIG_WBD),y)
OBJS += wbd_eap.o
endif
#endif // endif

OBJS += wlevent_eap.o
CFLAGS += -DBCM_WLEVENT

OBJS += wlceventd_eap.o
CFLAGS += -DBCM_WLCEVENTD

all: eapd

ifneq ($(PREBUILT_BCMBIN),1)
eapd: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
else
eapd:
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
endif


install: all
	install -d $(INSTALLDIR)/bin
	install eapd $(INSTALLDIR)/bin
	$(STRIP) $(INSTALLDIR)/bin/eapd

clean:
	rm -f *.o eapd
