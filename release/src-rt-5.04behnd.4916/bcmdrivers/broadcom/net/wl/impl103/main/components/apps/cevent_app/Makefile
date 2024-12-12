all:
	cp -f prebuilt/ceventc ceventc
	cp -f prebuilt/ceventd ceventd
	cp -f prebuilt/libceshared.so libceshared.so

install: all
	install -d $(INSTALLDIR)/usr/bin
	install ceventc $(INSTALLDIR)/usr/bin
	install ceventd $(INSTALLDIR)/usr/bin
	$(STRIP) $(INSTALLDIR)/usr/bin/ceventc
	$(STRIP) $(INSTALLDIR)/usr/bin/ceventd
	install -d $(INSTALLDIR)/usr/lib
	install libceshared.so $(INSTALLDIR)/usr/lib

clean:

