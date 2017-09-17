include ../../common.mak

CFLAGS += -Wall -O

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += $(EXTRA_FLAG)
endif

CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include
LDFLAGS = -L$(TOP)/shared -lshared -L$(TOP)/nvram${BCMEX}${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
INSTALL = install

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

OBJS = asuslog.o

ifneq ($(wildcard ./prebuild$(BCMEX)$(EXHND)),)
all:
	@-cp -f ./prebuild$(BCMEX)$(EXHND)/asuslog .
else
all: asuslog
endif

asuslog: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: all
	$(STRIP) asuslog
	$(INSTALL) -d $(INSTALLDIR)/sysstate/usr/sbin
	$(INSTALL) asuslog $(INSTALLDIR)/sysstate/usr/sbin

clean:
	rm -f asuslog *.o .*.depend

%.o: %.c .%.depend
	@echo " [asuslog] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
