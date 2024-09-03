all:

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install qosmgmtd $(INSTALLDIR)/usr/sbin/
	install qm_cli $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/qosmgmtd
	$(STRIP) $(INSTALLDIR)/usr/sbin/qm_cli

clean:

