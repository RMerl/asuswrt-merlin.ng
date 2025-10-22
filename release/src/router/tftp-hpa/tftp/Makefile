SRCROOT = ..
VERSION = $(shell cat ../version)

-include ../MCONFIG
include ../MRULES

OBJS = tftp.$(O) main.$(O)

all: tftp$(X) tftp.1

tftp$(X): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(TFTP_LIBS) -o $@

$(OBJS): ../common/tftpsubs.h

tftp.1: tftp.1.in ../version
	sed -e 's/@@VERSION@@/$(VERSION)/g' < $< > $@

install: all
	mkdir -p $(INSTALLROOT)$(BINDIR) $(INSTALLROOT)$(MANDIR)/man1
	$(INSTALL_PROGRAM) tftp$(X) $(INSTALLROOT)$(BINDIR)
	$(INSTALL_DATA)    tftp.1   $(INSTALLROOT)$(MANDIR)/man1

clean:
	rm -f *.o *.obj *.exe tftp tftp.1

distclean: clean
	rm -f *~
