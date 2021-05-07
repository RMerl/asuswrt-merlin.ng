# libupnp
include ../common.mak

UPNPLIB_NAME = libupnp.so

all:
	cp -f ./prebuilt/$(UPNPLIB_NAME) .

install: all
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 $(UPNPLIB_NAME) $(INSTALLDIR)/usr/lib
	$(STRIP) $(INSTALLDIR)/usr/lib/$(UPNPLIB_NAME)

clean:
	rm -f $(UPNPLIB_NAME) $(OBJFILES)

.PHONY: clean
