#
# Linux acsdv2 Makefile
#

all:
	-cp -f prebuilt/acsd2 acsd2
	-cp -f prebuilt/acs_cli2 acs_cli2

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install acsd2 $(INSTALLDIR)/usr/sbin/
	install acs_cli2 $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/acsd2
	$(STRIP) $(INSTALLDIR)/usr/sbin/acs_cli2

clean:
	rm -f acsd2 acs_cli2
