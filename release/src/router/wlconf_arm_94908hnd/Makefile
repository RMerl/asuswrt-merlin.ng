# wlconf
include $(SRCBASE)/router/.config

all:
	-cp -f $(SRCBASE)/router/wlconf$(BCMEX)$(EX7)/prebuilt/wlconf wlconf

clean:
	rm -f *.o wlconf

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install wlconf $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/wlconf
