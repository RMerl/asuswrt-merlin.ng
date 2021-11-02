include ../.config
include $(SRCBASE)/makefiles/WLAN_Common.mk

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/../components/shared -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/common/include -Wall
CFLAGS	+= $(WLAN_ComponentIncPathA)
CFLAGS  += -I$(SRCBASE)/shared/bcmwifi/include
CFLAGS  +=  -I$(HND_SRC)/bcmdrivers/broadcom/char/wlcsm_ext/impl1/include/
CFLAGS  +=  -I$(HND_SRC)/router-sysdep/wlcsm/include/
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s
ifeq ($(CONFIG_RTR_OPTIMIZE_SIZE),y)
CFLAGS  += -Os
else
CFLAGS  += -O2
endif
LDFLAGS	+= -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram ${EXTRA_NV_LDFLAGS} -L$(TOP)/shared -lshared -lpthread
LDFLAGS += -lgcc_s

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

vpath %.c $(SRCBASE)/shared $(WLAN_ComponentSrcDirs) $(SRCBASE)/shared/bcmwifi/src

OBJS := wlconf.o bcmwifi_channels.o

ifeq ($(wildcard $(TOP_PLATFORM)/wlconf$(BCMEX)$(EX7)/*.c),)
all:
	-cp -f $(TOP_PLATFORM)/wlconf$(BCMEX)$(EX7)/prebuilt/wlconf wlconf
else
all: wlconf
endif

clean:
	rm -f *.o wlconf

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install wlconf $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/wlconf

wlconf: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
