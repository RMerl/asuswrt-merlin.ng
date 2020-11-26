include $(SRCBASE)/router/.config

CFLAGS  += -I. -I$(TOP)/shared  -I$(SRCBASE)/include -Wall

LDFLAGS += -L$(TOP)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared

all: aura_rgb_nt aura_rgb_sw

clean:
	rm -f *.o *~ aura_rgb_nt aura_rgb_sw

install: all
	install -d $(INSTALLDIR)/usr/sbin
	$(STRIP) aura_rgb_nt
	install aura_rgb_nt $(INSTALLDIR)/usr/sbin
	$(STRIP) aura_rgb_sw
	install aura_rgb_sw $(INSTALLDIR)/usr/sbin

aura_rgb_nt: aura_rgb_nt.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
	$(STRIP) $@

aura_rgb_sw: aura_rgb_sw.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
	$(STRIP) $@