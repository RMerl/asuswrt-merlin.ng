include ../common.mak
include $(SRCBASE)/.config
include $(SRCBASE)/router/.config

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC

OBJS = main.o


INSTALL = install

ifeq ($(wildcard ./*.c),)
all:
	cp -f ./prebuild/dhcpfwd .
else
all: dhcpfwd
endif

dhcpfwd: $(OBJS)
	@echo " [dhcpfwd] LD $@"
	@$(CC) $(LDFLAGS) -o $@ $^

install: all
	@echo " [dhcpfwd] Installing as $(INSTALLDIR)/usr/sbin/dhcpfwd"
	@$(INSTALL) -d $(INSTALLDIR)/usr/sbin
	@$(INSTALL) -m 755 dhcpfwd $(INSTALLDIR)/usr/sbin/
	@$(STRIP) $(INSTALLDIR)/usr/sbin/dhcpfwd

clean:
	rm -f *.o *.so *.a .*.depend *.prep

%.o: %.c .%.depend
	@echo " [dhcpfwd] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

%.o: prebuild/%.o
	@echo " [dhcpfwd] cp $@"
	@cp -f $< $@

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

%.prep: %.c
	@$(CC) $(CFLAGS) -E $< > $@

-include $(OBJS:%.o=.%.depend)
