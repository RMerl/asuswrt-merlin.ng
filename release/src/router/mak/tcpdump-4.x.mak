tcpdump-4.x: libpcap tcpdump-4.x/Makefile
	@$(SEP)
	@$(MAKE) -j 8 -C tcpdump-4.x

tcpdump-4.x/Makefile:
	cd tcpdump-4.x && $(CONFIGURE) ac_cv_linux_vers=2

tcpdump-4.x-install: tcpdump-4.x
	install -D tcpdump-4.x/tcpdump $(INSTALLDIR)/tcpdump-4.x/usr/sbin/tcpdump
	$(STRIP) $(INSTALLDIR)/tcpdump-4.x/usr/sbin/tcpdump

tcpdump-4.x-clean:
	[ ! -f tcpdump-4.x/Makefile ] || $(MAKE) -C tcpdump-4.x distclean
