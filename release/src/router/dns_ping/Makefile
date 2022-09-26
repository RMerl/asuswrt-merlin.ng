include ../common.mak
include $(SRCBASE)/.config
include $(SRCBASE)/router/.config

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC

OBJS = dns_ping.o

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += $(EXTRA_FLAG)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include
LDFLAGS = -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} ${EXTRA_NV_LDFLAGS} -lnvram

CFLAGS  += -I$(TOP)/sqlite
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3

ifeq ($(RTCONFIG_LIB_CODB),y)
# json-c is required by libcodb
CFLAGS += -I$(TOP)/json-c
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
CFLAGS += -I$(TOP)/libcodb
LDFLAGS += -L$(TOP)/libcodb -lcodb
endif

ifeq ($(RTCONFIG_QCA),y)
CFLAGS += $(EXTRACFLAGS)
LDFLAGS += $(EXTRALDFLAGS)
endif

ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif

INSTALL = install

ifeq ($(wildcard ./*.c),)
all:
	cp -f ./prebuild/dns_ping .
else
all: dns_ping
endif

dns_ping: $(OBJS)
	@echo " [dns_ping] LD $@"
	@$(CC) $(LDFLAGS) -o $@ $^

install: all
	@echo " [dns_ping] Installing as $(INSTALLDIR)/usr/sbin/dns_ping"
	@$(INSTALL) -d $(INSTALLDIR)/usr/sbin
	@$(INSTALL) -m 755 dns_ping $(INSTALLDIR)/usr/sbin/
	@$(STRIP) $(INSTALLDIR)/usr/sbin/dns_ping

clean:
	rm -f *.o *.so *.a .*.depend *.prep

%.o: %.c .%.depend
	@echo " [dns_ping] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

%.o: prebuild/%.o
	@echo " [dns_ping] cp $@"
	@cp -f $< $@

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

%.prep: %.c
	@$(CC) $(CFLAGS) -E $< > $@

-include $(OBJS:%.o=.%.depend)
