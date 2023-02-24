include ${TOP}/common.mak

CFLAGS += -Wall -O

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += $(EXTRA_FLAG)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include
LDFLAGS += -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX}$(EX7) -lnvram ${EXTRA_NV_LDFLAGS} -lpthread
INSTALL = install

CFLAGS += -I$(TOP)/json-c
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c

ifeq ($(RTCONFIG_QTN),y)
CFLAGS += -I$(TOP)/libqcsapi_client -I$(TOP)/httpd -I./sysdeps/qtn
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif

ifneq ($(wildcard ./prebuild),)
all:
	@-cp -f ./prebuild/$(BUILD_NAME)/* .
else
all: fsmd req_fsm
endif

fsmd: fsmd.o jffs.o tmpfs.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

req_fsm: req_fsm.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: all
	$(STRIP) fsmd
	$(STRIP) req_fsm
	$(INSTALL) -d $(INSTALLDIR)/usr/sbin/
	$(INSTALL) fsmd $(INSTALLDIR)/usr/sbin/fsmd
	$(INSTALL) req_fsm $(INSTALLDIR)/usr/sbin/req_fsm

clean:
	rm -f fsmd req_fsm *.o .*.depend

%.o: %.c .%.depend
	@echo " [fsmd] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
