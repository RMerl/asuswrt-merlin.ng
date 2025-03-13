#
# $Copyright ASUSTeK Corporation$
#
# $Id: Makefile 401201 2013-05-09 03:06:42Z kenlo $
#

#include $(SRCBASE)/router/.config

include $(SRCBASE)/router/.config


CFLAGS = -s -Os -fomit-frame-pointer -fPIC
CFLAGS += -Wall -I. -I../httpd -I../shared 
CFLAGS += -I$(TOP)/shared -I$(SRCBASE) -I$(SRCBASE)/include -I$(SRCBASE)/common/include
CFLAGS += -I$(TOP)/json-c
CFLAGS += -I$(TOP)/cfg_mnt


LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -lpthread


#ifeq ($(HND_ROUTER), y)
#LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND)
#LIBS  += -lwlcsm
#endif


ifeq ($(AX), y)
    CFLAGS += -I$(SRCBASE)/../components/proto/include
    CFLAGS += -I$(SRCBASE)/../components/wlioctl/include
    CFLAGS += -I$(SRCBASE)/wl 
    CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
    CFLAGS += -I$(SRCBASE)/common/include

#    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) 
#    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
#    LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
else
    CFLAGS += -I$(HND_SRC)/bcmdrivers/opensource/include/bcm963xx
    CFLAGS += -I$(SRCBASE)/include
    CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
    CFLAGS += -I$(SRCBASE)/common/include

#    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) 
#    LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)
#    LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
endif



ifeq ($(wildcard $(SRCBASE)/router/asus_rbd/*.c),)
all:
	-cp -f prebuild/$(BUILD_NAME)/asus_rbd asus_rbd 
else
all: asus_rbd
	@echo LDFLAGS=$(LDFLAGS)
	@echo "=============================";
	@echo CFLAGS=$(CFLAGS)
	@echo "=============================";
	@echo RTCONFIG_HND_ROUTER_AX_675X=$(RTCONFIG_HND_ROUTER_AX_675X)
	@echo RTCONFIG_HND_ROUTER_AX_6756=$(RTCONFIG_HND_ROUTER_AX_6756)
endif

.c.o:
	$(CC) -c $*.c $(CFLAGS)

OBJS = asus_rbd.o sta.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install asus_rbd $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/asus_rbd

clean:
	rm -f *.o *.a asus_rbd

${OBJS}:

asus_rbd: ${OBJS}
	$(CC) -o asus_rbd $(OBJS) $(LDFLAGS)

