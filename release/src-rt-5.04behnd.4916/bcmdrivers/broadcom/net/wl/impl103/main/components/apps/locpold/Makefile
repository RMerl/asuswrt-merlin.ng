all:
	cp -f prebuilt/bin/locpold ./
	cp -f prebuilt/bin/liblocpold.so ./

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install locpold $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/locpold
	install -d $(INSTALLDIR)/usr/lib
	install liblocpold.so $(INSTALLDIR)/usr/lib

clean:

