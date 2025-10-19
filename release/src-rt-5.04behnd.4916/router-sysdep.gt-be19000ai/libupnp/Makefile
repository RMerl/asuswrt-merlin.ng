
include $(TOP)/common.mak
UPNPLIB_NAME = libupnp.so

OBJFILES = ${SRCFILES:.c=.o}

all:
	echo use prebuilt 
	cp -f ./prebuilt/$(UPNPLIB_NAME) .

install:
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 $(UPNPLIB_NAME) $(INSTALLDIR)/usr/lib
	$(STRIP) $(INSTALLDIR)/usr/lib/$(UPNPLIB_NAME)

dynamic:

clean:
	rm -f $(UPNPLIB_NAME) $(OBJFILES)


