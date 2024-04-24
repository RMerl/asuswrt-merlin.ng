all:
	cp -f prebuild/$(BUILD_NAME)/dnsqd dnsqd
	cp -f prebuild/$(BUILD_NAME)/libdns_app_client.so libdns_app_client.so

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install dnsqd $(INSTALLDIR)/usr/sbin
	$(STRIP) $(INSTALLDIR)/usr/sbin/dnsqd
	install -d $(INSTALLDIR)/usr/lib
	install libdns_app_client.so $(INSTALLDIR)/usr/lib

clean:

