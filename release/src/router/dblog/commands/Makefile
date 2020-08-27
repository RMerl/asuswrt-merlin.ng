include ../../common.mak

CFLAGS += -Wall -O

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += $(EXTRA_FLAG)
endif

CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif
LDFLAGS = -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
INSTALL = install

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

OBJS = dblogcmd.o

ifneq ($(wildcard ./prebuild/$(BUILD_NAME)),)
all:
	@-cp -f ./prebuild/$(BUILD_NAME)/dblogcmd .
else
all: dblogcmd
endif

dblogcmd: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: all
	$(STRIP) dblogcmd
	$(INSTALL) -d $(INSTALLDIR)/dblog/usr/sbin
	$(INSTALL) dblogcmd $(INSTALLDIR)/dblog/usr/sbin

clean:
	rm -f dblogcmd *.o .*.depend

%.o: %.c .%.depend
	@echo " [dblogcmd] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
