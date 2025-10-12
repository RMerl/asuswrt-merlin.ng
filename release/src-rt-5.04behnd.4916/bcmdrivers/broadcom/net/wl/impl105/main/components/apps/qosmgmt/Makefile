all: qm_cli qosmgmtd

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install qosmgmtd $(INSTALLDIR)/usr/sbin/
	install qm_cli $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/qosmgmtd
	$(STRIP) $(INSTALLDIR)/usr/sbin/qm_cli

clean:

qosmgmtd:
	cp -f $(SRCBASE)/../components/apps/qosmgmt/prebuilt/$(BUILD_NAME)/$@ ./
qm_cli:
	cp -f $(SRCBASE)/../components/apps/qosmgmt/prebuilt/$(BUILD_NAME)/$@ ./
