# Router dhd_monitor Makefile
include $(TOP)/.config

all: debug_monitor

clean:
	rm -f *.o debug_monitor .*.depend

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install debug_monitor $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/debug_monitor

debug_monitor:
	cp -f $(TOP_PLATFORM)/$(shell pwd | sed 's/\(.*router-sysdep\/\)//')/prebuilt/$@ ./

