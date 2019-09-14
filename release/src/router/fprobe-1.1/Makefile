#include ../.config

CFLAGS  += -I. -I$(TOP)/libpcap -Dlinux -pthread
LDFLAGS = -L$(TOP)/libpcap -lpcap -lpthread

all: fprobe

clean:
	rm -f *.o fprobe

install: all
	echo $(INSTALLDIR)
	install -d $(INSTALLDIR)/usr/sbin
	$(STRIP) fprobe
	install fprobe $(INSTALLDIR)/usr/sbin


fprobe: fprobe.c hash.c mem.c my_getopt.c my_log.c netflow.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
	$(STRIP) $@
