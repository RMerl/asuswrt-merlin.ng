include $(SRCBASE)/router/.config

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall
CFLAGS	+= $(WLAN_ComponentIncPath)
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s
ifeq ($(CONFIG_RTR_OPTIMIZE_SIZE),y)
CFLAGS  += -Os
else
CFLAGS  += -O2
endif
LDFLAGS	+= -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram ${EXTRA_NV_LDFLAGS} -L$(TOP)/shared -lshared
LDFLAGS += -lgcc_s

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(RTCONFIG_AMAS),y)
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
endif

vpath %.c $(SRCBASE)/shared $(WLAN_ComponentSrcDirs)

OBJS := wlconf.o bcmwifi_channels.o

ifeq ($(wildcard $(SRCBASE)/router/wlconf$(BCMEX)$(EX7)/*.c),)
all:
	-cp -f $(SRCBASE)/router/wlconf$(BCMEX)$(EX7)/prebuilt/wlconf wlconf
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
