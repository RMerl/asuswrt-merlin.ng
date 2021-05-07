# acsd
include ../.config

ifeq ($(wildcard $(SRCBASE)/router/acsd${BCMEX}/*.c),)
all:
	-cp -f prebuilt/acsd acsd
endif

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install acsd $(INSTALLDIR)/usr/sbin/
#	install acs_cli $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/acsd
#	$(STRIP) $(INSTALLDIR)/usr/sbin/acs_cli

clean:
	rm -f *.o *.a *.so acsd acs_cli
