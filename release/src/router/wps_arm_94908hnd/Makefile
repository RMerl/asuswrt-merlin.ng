# WPS

include $(TOP)/.config

# linux kernel config
include $(LINUXDIR)/.config

all: wps_monitor

wps_monitor :

clean:
	find . \( -name '*.[od]' -o -name '*.tmp' \) -type f -print | xargs rm -f
	find -name obj | xargs rm -rf
	rm -rf lib

# install
install: all
	install -d $(INSTALLDIR)/wps${BCMEX}$(EX7)/bin
	install prebuilt/wps_monitor $(INSTALLDIR)/wps${BCMEX}$(EX7)/bin
	$(STRIP) $(INSTALLDIR)/wps${BCMEX}$(EX7)/bin/wps_monitor

.PHONY: all clean install
