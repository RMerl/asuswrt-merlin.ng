#
# libupnp

include $(TOP)/common.mak
UPNPLIB_NAME = libupnp.so

UPNPLIB = $(SRCBASE)/router-sysdep/libupnp
SRCPATH = $(UPNPLIB)/upnp

vpath %.c $(SRCPATH) $(UPNPLIB)/linux
vpath %.o $(UPNPLIB)/prebuilt

all:
	echo use prebuilt 
	cp -f ./prebuilt/$(UPNPLIB_NAME) .


install: all
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 $(UPNPLIB_NAME) $(INSTALLDIR)/usr/lib
	$(STRIP) $(INSTALLDIR)/usr/lib/$(UPNPLIB_NAME)

clean:
	rm -f $(UPNPLIB_NAME) $(OBJFILES)

.PHONY: clean
