all:
	cp -f prebuilt/$(BUILD_NAME)/afc_cli ./
	cp -f prebuilt/$(BUILD_NAME)/afcd ./
	cp -f prebuilt/$(BUILD_NAME)/libafc.so ./

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install afc_cli $(INSTALLDIR)/usr/sbin
	install afcd $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/afc_cli
	$(STRIP) $(INSTALLDIR)/usr/sbin/afcd
	install -d $(INSTALLDIR)/usr/lib
	install libafc.so $(INSTALLDIR)/usr/lib

clean:

