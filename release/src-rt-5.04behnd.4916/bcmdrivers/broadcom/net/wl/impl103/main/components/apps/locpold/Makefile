all:
	cp -f prebuilt/$(BUILD_NAME)/bin/locpold ./
	cp -f prebuilt/$(BUILD_NAME)/bin/liblocpol.so ./

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install locpold $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/locpold
	install -d $(INSTALLDIR)/usr/lib
	install liblocpol.so $(INSTALLDIR)/usr/lib

clean:

