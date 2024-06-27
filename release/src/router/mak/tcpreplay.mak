tcpreplay: libpcap-1.9.1 tcpreplay/Makefile
	@$(SEP)
	@$(MAKE) -j4 -C $@

tcpreplay/configure:
	cd tcpreplay && ./autogen.sh

tcpreplay/Makefile: tcpreplay/configure
	cd tcpreplay && \
		$(CONFIGURE) --prefix=/usr \
		--enable-dynamic-link --enable-tcpreplay-edit --enable-local-libopts \
		--disable-libopts-install --disable-maintainer-mode \
		CFLAGS="-I$(STAGEDIR)/usr/include" LDFLAGS="-L$(STAGEDIR)/usr/lib" \
		with_libpcap=$(STAGEDIR)/usr

tcpreplay-install: tcpreplay
	install -D tcpreplay/src/tcpreplay $(INSTALLDIR)/tcpreplay/usr/sbin/tcpreplay
	$(STRIP) $(INSTALLDIR)/tcpreplay/usr/sbin/tcpreplay

tcpreplay-clean:
	[ ! -f tcpreplay/Makefile ] || $(MAKE) -C tcpreplay distclean
	@find tcpreplay -name .deps -type d|xargs rm -fr
	@[ ! -e tcpreplay/Makefile ] || rm -f tcpreplay/Makefile
	@[ ! -e tcpreplay/configure ] || rm -f tcpreplay/configure
