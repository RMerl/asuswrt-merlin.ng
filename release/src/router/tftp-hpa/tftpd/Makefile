SRCROOT = ..
VERSION = $(shell cat ../version)

-include ../MCONFIG
include ../MRULES

OBJS = tftpd.$(O) recvfrom.$(O) misc.$(O) $(TFTPDOBJS)

all: tftpd$(X) tftpd.8

tftpd$(X): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(TFTPD_LIBS) -o $@

$(OBJS): ../common/tftpsubs.h

tftpd.8: tftpd.8.in ../version
	sed -e 's/@@VERSION@@/$(VERSION)/g' < $< > $@

install: all
	mkdir -p $(INSTALLROOT)$(SBINDIR) $(INSTALLROOT)$(MANDIR)/man8
	$(INSTALL_PROGRAM) tftpd$(X) $(INSTALLROOT)$(SBINDIR)/in.tftpd
	$(INSTALL_DATA)    tftpd.8 $(INSTALLROOT)$(MANDIR)/man8/in.tftpd.8
	cd $(INSTALLROOT)$(MANDIR)/man8 && $(LN_S) -f in.tftpd.8 tftpd.8

clean:
	rm -f *.o *.obj *.exe tftpd tftpsubs.c tftpsubs.h tftpd.8

distclean: clean
	rm -f *~
