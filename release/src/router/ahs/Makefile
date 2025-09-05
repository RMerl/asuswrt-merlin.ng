include ${TOP}/common.mak

CFLAGS += -Wall -O $(if $(RTCONFIG_DEBUG),-g)

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += $(EXTRA_FLAG)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include
LDFLAGS = -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX}$(EX7) -lnvram ${EXTRA_NV_LDFLAGS} -lpthread
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif
INSTALL = install

ifeq ($(RTCONFIG_HTTPS),y)
CFLAGS += -I$(TOP)/curl/include
LDFLAGS += -L$(TOP)/curl/lib/.libs -lcurl
CFLAGS += -I$(TOP)/openssl/include
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl $(if $(RTCONFIG_OPENSSL11),-lpthread,)
ifeq ($(RTCONFIG_TR069),y)
CFLAGS += -I$(TOP)/libssh2/include -I$(TOP)/zlib/include
LDFLAGS += -L$(TOP)/libssh2/src/.libs -lssh2 -L$(TOP)/zlib -lz
endif
endif

CFLAGS += -I$(TOP)/json-c
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c

CFLAGS += -I$(TOP)/libasc
LDFLAGS += -L$(TOP)/libasc -lasc

ifeq ($(RTCONFIG_QTN),y)
CFLAGS += -I$(TOP)/libqcsapi_client -I$(TOP)/httpd -I./sysdeps/qtn
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(HND_ROUTER_BE_4916), y)
CFLAGS+= -fcommon
endif

ifeq ($(RTCONFIG_TRUSTZONE),y)
LDFLAGS += -L$(TOP)/ateed/ -latee
endif

OBJS = utility.o ahs.o

ifneq ($(wildcard ./prebuild),)
all:
	@-cp -f ./prebuild/$(BUILD_NAME)/ahs .
else
all: ahs
endif

ahs: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: all
	$(if $(RTCONFIG_DEBUG),,$(STRIP) ahs)
	$(INSTALL) -d $(INSTALLDIR)/ahs/usr/sbin
	$(INSTALL) ahs $(INSTALLDIR)/ahs/usr/sbin

clean:
	rm -f ahs *.o .*.depend

%.o: %.c .%.depend
	@echo " [ahs] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
