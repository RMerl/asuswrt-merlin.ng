include ../../common.mak

CFLAGS += -Wall -O


ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += $(EXTRA_FLAG)
endif

CFLAGS += -DASUS
CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

ifeq ($(HND_ROUTER_BE_4916), y)
CFLAGS+= -fcommon
endif

LDFLAGS = -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif
INSTALL = install

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(RTCONFIG_TRUSTZONE),y)
LDFLAGS += -L$(TOP)/ateed/ -latee
endif

OBJS = dblog.o dblog_syslog.o dblog_wifi.o dblog_dm.o dblog_ms.o dblog_amas.o dblog_hyd.o respipc.o

ifneq ($(wildcard ./prebuild/$(BUILD_NAME)),)
all:
	@-cp -f ./prebuild/$(BUILD_NAME)/dblog .
else
all: dblog
endif

dblog: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: all
	$(STRIP) dblog
	$(INSTALL) -d $(INSTALLDIR)/dblog/usr/sbin
	$(INSTALL) dblog $(INSTALLDIR)/dblog/usr/sbin

clean:
	rm -f dblog *.o .*.depend

%.o: %.c .%.depend
	@echo " [dblog] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
