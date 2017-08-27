include ../common.mak
include $(SRCBASE)/.config

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include
CFLAGS += -I$(TOP)/openssl/include
LDFLAGS += $(EXTRALDFLAGS) -L$(TOP)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -lcrypt
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl

INSTALL = install

OBJS = openvpn_config.o openvpn_control.o

ifeq ($(wildcard ./*.c),)
all:
	cp -f ./prebuild/libvpn.so .
else
all: libvpn.so
endif

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

%.o: prebuild/%.o
	@echo " [libvpn] cp $@"
	@cp -f $< $@

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

%.prep: %.c
	@$(CC) $(CFLAGS) -E $< > $@

-include $(OBJS:%.o=.%.depend)
