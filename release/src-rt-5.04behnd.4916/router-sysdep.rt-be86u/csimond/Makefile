PROGS     = csimond

include $(SRCBASE_ROUTER)/.config

INSTDIR   = $(INSTALLDIR)/usr/sbin/
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root
#Add LDFLAGS and CFLAGS to include bcmshared, nvram libraries used by csimond
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += -L$(TOP)/libbcm -L$(INSTALLDIR)/libbcm/usr/lib -lbcm
ifeq ($(strip $(BCA_CPEROUTER)),)
#LDFLAGS	+= -L$(TOP)/nvram -L$(PLATFORMDIR)/install/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm
LDFLAGS += -L$(TOP_PLATFORM)/nvram -L$(PLATFORMDIR)/install/nvram/usr/lib -lnvram -lpthread
endif
ifneq ($(CMWIFI),)
CFLAGS += -I$(WL_CMWIFI_DIR)/libs/wlcsm/include
endif
LDFLAGS += $(EXTRA_LDFLAGS)
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-BE82M GS-BE18000 GS-BE12000 GS7_PRO GT7))
LDFLAGS += -L$(TOP)/libhostapi -lhostapi
endif

OBJS      = csimond.o

INSTALL   = install

all: $(PROGS)

CFLAGS   += -DSHELLPATH=\"$(run_prefix)/bin/sh\" -Wall -Os
#CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/../components/shared -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/common/include
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(SRCBASE)/../components/shared -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/common/include -I$(SRCBASE)/shared/bcmwifi/include/


CFLAGS += $(if $(WLAN_ComponentIncPathA),$(WLAN_ComponentIncPathA),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
ifneq ($(PREBUILT_BCMBIN),1)
$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
else
$(PROGS): $(OBJS)
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
endif

.PHONY: install
install: $(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) $(PROGS) $(INSTDIR)
	$(STRIP) $(foreach prog,$(PROGS),$(INSTDIR)/$(prog))

.PHONY: clean
clean:
	rm -f $(PROGS) *.o core
