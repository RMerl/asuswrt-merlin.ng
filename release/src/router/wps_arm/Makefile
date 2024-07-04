include $(TOP)/.config

all:

install: all
	install -d $(INSTALLDIR)/wps${BCMEX}/bin
	install prebuilt/wps_monitor $(INSTALLDIR)/wps${BCMEX}/bin
	$(STRIP) $(INSTALLDIR)/wps${BCMEX}/bin/wps_monitor
