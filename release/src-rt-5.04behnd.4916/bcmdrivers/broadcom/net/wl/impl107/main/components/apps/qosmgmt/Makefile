all: prebuilt

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install qosmgmtd $(INSTALLDIR)/usr/sbin/
	install qm_cli $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/qosmgmtd
	$(STRIP) $(INSTALLDIR)/usr/sbin/qm_cli

prebuilt:
	cp -f $(SRCBASE)/../components/apps/qosmgmt/prebuilt/$(BUILD_NAME)/* ./

clean:

