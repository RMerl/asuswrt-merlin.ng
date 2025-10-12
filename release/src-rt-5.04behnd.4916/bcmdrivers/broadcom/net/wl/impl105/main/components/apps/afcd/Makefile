all:
	cp -f prebuilt/afc_cli ./
	cp -f prebuilt/afcd ./
	cp -f prebuilt/libafc.so ./

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install afc_cli $(INSTALLDIR)/usr/sbin
	install afcd $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/afc_cli
	$(STRIP) $(INSTALLDIR)/usr/sbin/afcd
	install -d $(INSTALLDIR)/usr/lib
	install libafc.so $(INSTALLDIR)/usr/lib

clean:

