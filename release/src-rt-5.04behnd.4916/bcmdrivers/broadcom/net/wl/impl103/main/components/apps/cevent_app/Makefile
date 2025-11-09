all:
	cp -f prebuilt/ceventc ceventc
	cp -f prebuilt/ceventd ceventd
	cp -f prebuilt/libceshared.so libceshared.so

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install ceventc $(INSTALLDIR)/usr/sbin
	install ceventd $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/ceventc
	$(STRIP) $(INSTALLDIR)/usr/sbin/ceventd
	install -d $(INSTALLDIR)/usr/lib
	install libceshared.so $(INSTALLDIR)/usr/lib

clean:

