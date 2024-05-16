all:
	cp prebuilt/wlconf ./wlconf

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install wlconf $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/wlconf

dynamic:

clean:
	rm -f *.o wlconf


