all:

install: all
	install -D prebuild/wapp $(INSTALLDIR)/bin/wapp
	install -D prebuild/wappctrl $(INSTALLDIR)/bin/wappctrl
	$(STRIP) $(INSTALLDIR)/bin/wapp
	$(STRIP) $(INSTALLDIR)/bin/wappctrl
	chmod 0500 $(INSTALLDIR)/bin/wapp
	chmod 0500 $(INSTALLDIR)/bin/wappctrl
	install -D -m 755 prebuild/libkvcutil.so  $(INSTALLDIR)/usr/lib/libkvcutil.so
	$(STRIP) $(INSTALLDIR)/usr/lib/libkvcutil.so
	@cd $(INSTALLDIR)/usr/lib && ln -sf libkvcutil.so libkvcutil.so.1

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
