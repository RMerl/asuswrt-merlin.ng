mcu_DATA = mcu/app.1.1.116.hex
mcu_DATA += mcu/app.1.2.116.hex
mcu_DATA += mcu/app.2.2.120.bin

all:

install:
	install -d $(INSTALLDIR)/mcu
	install -d $(INSTALLDIR)/usr/lib
	install -d $(INSTALLDIR)/usr/sbin
	install -m 0644 -t $(INSTALLDIR)/mcu $(mcu_DATA)
	install -m 0644 -t $(INSTALLDIR)/usr/lib libwlcr.so
	install -m 0755 -t $(INSTALLDIR)/usr/sbin uhmi
	install -m 0755 -t $(INSTALLDIR)/usr/sbin uhmiMsg
	install -m 0755 -t $(INSTALLDIR)/usr/sbin phi_speed
	install -m 0755 -t $(INSTALLDIR)/usr/sbin wl_cr

clean:

distclean:

