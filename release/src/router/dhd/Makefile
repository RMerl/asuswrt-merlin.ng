all:
	-cp -f prebuilt$(BCMEX)$(EX7)$(EXHND)/dhd dhd
clean:
	rm -f *.o dhd

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install dhd $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/dhd

dhd:
	-cp -f prebuilt$(BCMEX)$(EX7)$(EXHND)/dhd dhd

