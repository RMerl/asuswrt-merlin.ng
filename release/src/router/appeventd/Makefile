# Linux appeventd Makefile

include $(TOP)/.config

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall -Werror
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
ifeq ($(RTCONFIG_HND_ROUTER_AX),y)
CFLAGS += -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/math/include
endif
LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += $(EXTRA_LDFLAGS) -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm

vpath %.c $(SRCBASE)/shared/

ifeq ($(wildcard $(SRCBASE)/router/appeventd/*.c),)
all:
	-cp -f prebuilt/appeventd appeventd
else
all: appeventd
endif


OBJS = appeventd_main.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install appeventd $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/appeventd

clean:
	rm -f *.o *.a *.so appeventd

${OBJS}: appeventd.h

appeventd: bcmutils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)
