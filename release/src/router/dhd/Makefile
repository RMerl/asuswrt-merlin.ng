ifeq ($(HND_ROUTER),y)
all clean install:
	@true	#Do nothing
else
all:
	-cp -f prebuilt$(BCMEX)$(EX7)/dhd dhd
clean:
	rm -f *.o dhd

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install dhd $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/dhd

dhd:
	-cp -f prebuilt$(BCMEX)$(EX7)/dhd dhd
endif

