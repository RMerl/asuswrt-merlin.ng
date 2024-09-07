include $(TOP)/.config

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall -Werror
ifeq ($(RTCONFIG_HND_ROUTER_AX),y)
CFLAGS += -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/math/include
endif
CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall -Werror
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram ${EXTRA_NV_LDFLAGS}
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif
LDFLAGS += $(EXTRA_LDFLAGS)


ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-BE82M GS-BE18000))
LDFLAGS += -L$(TOP)/libhostapi -lhostapi
endif

vpath %.c $(SRCBASE)/shared/

ifeq ($(wildcard $(SRCBASE)/router/eventd/*.c),)
all:
	-cp -f prebuilt/$(BUILD_NAME)/eventd eventd
else
all: eventd
endif

OBJS = eventd_main.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install eventd $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/eventd

clean:
	rm -f *.o *.a *.so eventd

${OBJS}: eventd.h

eventd: bcmutils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
