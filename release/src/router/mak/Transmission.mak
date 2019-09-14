Transmission-configure:
	( cd Transmission && ./autogen.sh && \
		$(CONFIGURE) --prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib \
			CFLAGS="$(CFLAGS) -I$(STAGEDIR)/usr/include" \
			LDFLAGS="$(LDFLAGS) -L$(STAGEDIR)/usr/lib" \
			--disable-nls --disable-gtk \
	)

Transmission/Makefile:
	$(MAKE) Transmission-configure

Transmission: curl-7.21.7 libevent-2.0.21 Transmission/Makefile
	@$(SEP)
	$(MAKE) -C $@

Transmission-install: Transmission
	install -D $</daemon/transmission-daemon $(INSTALLDIR)/$</usr/sbin/transmission-daemon
	install -D $</daemon/transmission-remote $(INSTALLDIR)/$</usr/sbin/transmission-remote
	$(STRIP) $(INSTALLDIR)/$</usr/sbin/*

Transmission-clean:
	[ ! -f Transmission/Makefile ] || $(MAKE) -C Transmission KERNEL_DIR=$(LINUX_INC_DIR) distclean
	@rm -f Transmission/Makefile
