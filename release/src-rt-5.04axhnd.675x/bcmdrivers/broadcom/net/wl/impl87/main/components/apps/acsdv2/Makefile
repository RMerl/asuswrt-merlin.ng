all: acsd2 acs_cli2

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install acsd2 $(INSTALLDIR)/usr/sbin/
	install acs_cli2 $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/acsd2
	$(STRIP) $(INSTALLDIR)/usr/sbin/acs_cli2

clean:
	rm -f *.o *.a *.so acsd2 acs_cli2

acsd2:
	cp -f $(SRCBASE)/../components/apps/acsdv2/prebuilt/$(BUILD_NAME)/$@ ./
acs_cli2:
	cp -f $(SRCBASE)/../components/apps/acsdv2/prebuilt/$(BUILD_NAME)/$@ ./
