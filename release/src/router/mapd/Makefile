all:

install: all
	install -D prebuild/bs20 $(INSTALLDIR)/bin/bs20
	$(STRIP) $(INSTALLDIR)/bin/bs20
	chmod 0500 $(INSTALLDIR)/bin/bs20
	install -D prebuild/mapd_cli $(INSTALLDIR)/bin/mapd_cli
	$(STRIP) $(INSTALLDIR)/bin/mapd_cli
	chmod 0500 $(INSTALLDIR)/bin/mapd_cli
	install -D -m 755 prebuild/libmapd_interface_client.so $(INSTALLDIR)/usr/lib/libmapd_interface_client.so
	$(STRIP) $(INSTALLDIR)/usr/lib/libmapd_interface_client.so

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
