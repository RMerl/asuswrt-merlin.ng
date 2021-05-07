# wlconf
include $(SRCBASE)/router/.config

all:
	-cp -f $(SRCBASE)/router/wlconf$(BCMEX)/prebuilt/wlconf wlconf

clean:
	rm -f *.o wlconf

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install wlconf $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/wlconf
