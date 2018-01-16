include ../common.mak
include $(SRCBASE)/.config
include $(SRCBASE)/router/.config

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include
CFLAGS += -I$(TOP)/openssl/include
LDFLAGS += $(EXTRALDFLAGS) -L$(TOP)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -lcrypt
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl


INSTALL = install

OBJS = letsencrypt_config.o letsencrypt_control.o letsencrypt_utils.o

ifeq ($(wildcard ./*.c),)
all:
	cp -f ./prebuild/libletsencrypt.so .
else
all: libletsencrypt.so
endif

libletsencrypt.so: $(OBJS)
	@echo " [libletsencrypt] LD $@"
	@$(CC) $(LDFLAGS) -shared -o $@ $^


install: all
	@echo " [libletsencrypt] Installing as $(INSTALLDIR)/usr/lib/libletsencrypt.so"
	@install -d $(INSTALLDIR)/usr/lib
	@install -m 755 libletsencrypt.so $(INSTALLDIR)/usr/lib
	@$(STRIP) $(INSTALLDIR)/usr/lib/libletsencrypt.so

clean:
	rm -f *.o *.so *.a .*.depend *.prep

%.o: %.c .%.depend
	@echo " [libletsencrypt] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

%.o: prebuild/%.o
	@echo " [libletsencrypt] cp $@"
	@cp -f $< $@

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

%.prep: %.c
	@$(CC) $(CFLAGS) -E $< > $@

-include $(OBJS:%.o=.%.depend)
