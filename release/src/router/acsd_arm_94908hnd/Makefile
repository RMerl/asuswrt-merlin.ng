# Linux acsd Makefile

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS	+= -g
CFLAGS	+= $(EXTRACFLAGS)
#CFLAGS	+= -DACS_DEBUG
#CFLAGS	+= -s -O2
CFLAGS  += -DWL11AC_80P80 -DWL11AC_160
LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -L$(INSTALLDIR)/nvram/usr/lib -lnvram ${EXTRA_NV_LDFLAGS}
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += -lgcc_s

include ../.config
ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

vpath %.c $(SRCBASE)/shared/ $(SRCBASE)/shared/bcmwifi/src

ifeq ($(wildcard $(SRCBASE)/router/acsd${BCMEX}$(EX7)/*.c),)
all:
	-cp -f prebuilt/acsd acsd
else
all: acsd
endif

OBJS = bcmwifi_channels.o dcs.o acs.o acsd_utils.o acsd_main.o acsd_cmd.o acsd_chanim.o acs_dfsr.o
OBJS += chan_utils.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install acsd $(INSTALLDIR)/usr/sbin/
	#install acs_cli $(INSTALLDIR)/usr/sbin/
#	$(STRIP) $(INSTALLDIR)/usr/sbin/acsd
	#$(STRIP) $(INSTALLDIR)/usr/sbin/acs_cli

clean:
	rm -f *.o *.a *.so acsd acs_cli

${OBJS}: acsd.h acsd_svr.h acs_dfsr.h

acsd: bcmutils.o bcmxtlv.o bcm_app_utils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

acs_cli: acsd_cli.o acsd_utils.o bcmwifi_channels.o
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
