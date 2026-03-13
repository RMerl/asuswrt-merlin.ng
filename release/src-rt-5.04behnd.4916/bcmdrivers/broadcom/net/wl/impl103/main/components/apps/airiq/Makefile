include $(SRCBASE)/platform.mak
export SWSAVER = 2.6
SWSALIB = libairiq
SWSALIB_SO = $(SWSALIB).so.$(SWSAVER)
SIRPLIB = libairiq_sirp
SIRPLIB_SO = $(SIRPLIB).so.$(SWSAVER)
export PLATFORM=$(PLATFORM_ARCH)

all:

install:
	install -d $(INSTALLDIR)/usr/sbin
	install -m 755 ./prebuilt/$(PLATFORM)/airiq_app $(INSTALLDIR)/usr/sbin
	install -m 755 ./prebuilt/$(PLATFORM)/airiq_service $(INSTALLDIR)/usr/sbin
	install -m 555 ./prebuilt/$(PLATFORM)/airiq_service.cfg $(INSTALLDIR)/usr/sbin
	install -m 555 ./prebuilt/$(PLATFORM)/flash_policy.xml $(INSTALLDIR)/usr/sbin
	# Install Air-IQ libraries and .cfg file
	install -d $(INSTALLDIR)/lib
	install -m 755  ./prebuilt/$(PLATFORM)/$(SWSALIB_SO) $(INSTALLDIR)/lib/$(SWSALIB_SO)
	ln -sf ./$(SWSALIB_SO) $(INSTALLDIR)/lib/libairiq.so
	install -m 755  ./prebuilt/$(PLATFORM)/$(SIRPLIB_SO) $(INSTALLDIR)/lib/$(SIRPLIB_SO)
	ln -sf ./$(SIRPLIB_SO) $(INSTALLDIR)/lib/libairiq_sirp.so
	install -d $(INSTALLDIR)/etc
	install -m 555 ./prebuilt/$(PLATFORM)/airiq.cfg $(INSTALLDIR)/etc

clean:

