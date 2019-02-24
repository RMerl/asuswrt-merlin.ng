include ../common.mak
include $(SRCBASE)/.config

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include

ifeq ($(RTCONFIG_OPENSSL11),y)
CFLAGS += -I$(TOP)/openssl-1.1.x/include
LDFLAGS += -L$(TOP)/openssl-1.1.x -lcrypto -lssl
else
CFLAGS += -I$(TOP)/openssl/include
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
endif

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif
LDFLAGS += $(EXTRALDFLAGS) -L$(TOP)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -lcrypt

INSTALL = install

OBJS = openvpn_config.o openvpn_control.o

all: libvpn.so

libvpn.so: $(OBJS)
	@echo " [libvpn] LD $@"
	@$(CC) $(LDFLAGS) -shared -o $@ $^


install: all
	@echo " [libvpn] Installing as $(INSTALLDIR)/usr/lib/libvpn.so"
	@install -d $(INSTALLDIR)/usr/lib
	@install -m 755 libvpn.so $(INSTALLDIR)/usr/lib
	@$(STRIP) $(INSTALLDIR)/usr/lib/libvpn.so

clean:
	rm -f *.o *.so *.a .*.depend *.prep

%.o: %.c .%.depend
	@echo " [libvpn] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

%.prep: %.c
	@$(CC) $(CFLAGS) -E $< > $@

-include $(OBJS:%.o=.%.depend)
