include ../../common.mak

CFLAGS += -Wall -O


ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += $(EXTRA_FLAG)
endif

CFLAGS += -DASUS
CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include
LDFLAGS = -L$(TOP)/shared -lshared -L$(TOP)/nvram${BCMEX}${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
INSTALL = install

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

OBJS = dblog.o dblog_syslog.o dblog_wifi.o dblog_dm.o dblog_ms.o dblog_amas.o respipc.o

ifneq ($(wildcard ./prebuild$(BCMEX)$(EXHND)),)
all:
	@-cp -f ./prebuild$(BCMEX)$(EXHND)/dblog .
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
