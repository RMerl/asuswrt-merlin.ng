stress-1.x: stress-1.x/Makefile
	$(MAKE) -C stress-1.x

stress-1.x/Makefile:
	$(MAKE) stress-1.x-configure

stress-1.x-configure:
	( cd stress-1.x ; \
		./autogen.sh && $(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
	)

stress-1.x-install:
	install -D stress-1.x/src/stress $(INSTALLDIR)/stress-1.x/usr/sbin/stress
	$(STRIP) $(INSTALLDIR)/stress-1.x/usr/sbin/stress

stress-1.x-clean:
	[ ! -f stress-1.x/Makefile ] || $(MAKE) -C stress-1.x clean
	@rm -f stress-1.x/Makefile
