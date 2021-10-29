WLAN_ComponentsInUse += bcmwifi math
include $(SRCBASE)/makefiles/WLAN_Common.mk
export SRCBASE := $(WLAN_SrcBaseA)
export BASEDIR := $(WLAN_TreeBaseA)
#export TOP = ..
CFLAGS	+= $(EXTRACFLAGS)
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall -Werror
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include $(SRCBASE)/../components/math/include)))
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2 -fno-strict-aliasing
CFLAGS  += -DWL11AC_80P80 -DWL11AC_160
LDFLAGS += -L$(TOP_PLATFORM)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared -lpthread
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm
LDFLAGS += $(EXTRA_LDFLAGS)

CFLAGS  += -I$(SRCBASE)/../components/math/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/shared/bcmwifi/include -I$(SRCBASE)/../components/wlioctl/include

vpath %.c $(SRCBASE)/shared
vpath %.c $(SRCBASE)/shared/bcmwifi/src $(SRCBASE)/../components/math/src
vpath %.c $(SRCBASE)/../components/wlioctl/include $(SRCBASE)/include

all: acsd acs_cli

OBJS = bcmwifi_channels.o dcs.o acs_dfs.o acs_channels.o acs_cfg.o acs_scan.o acs_bgdfs.o acs_iov.o acs_fcs.o acs.o acsd_utils.o acsd_main.o acsd_cmd.o acsd_chanim.o acs_dfsr.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install acsd $(INSTALLDIR)/usr/sbin/
	install acs_cli $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/acsd
	$(STRIP) $(INSTALLDIR)/usr/sbin/acs_cli

clean:
	rm -f *.o *.a *.so acsd acs_cli

${OBJS}: acsd.h acsd_svr.h acs_dfsr.h

ifneq ($(PREBUILT_BCMBIN),1)
acsd: bcmutils.o bcmxtlv.o bcm_app_utils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

acs_cli: acsd_cli.o acsd_utils.o bcmwifi_channels.o
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
else
acsd:
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
acs_cli:
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/$(PRBM_DIR)/$@ ./
endif
