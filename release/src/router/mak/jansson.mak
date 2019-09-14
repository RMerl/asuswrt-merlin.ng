jansson/configure jansson/missing:
	cd jansson && ./autogen.sh

jansson/Makefile: jansson/configure
	cd jansson && $(CONFIGURE) \
		--prefix=/usr --bindir=/usr/sbin --libdir=/usr/lib \
		LDFLAGS="$(EXTRACFLAGS) -lm"

jansson: libdaemon jansson/Makefile jansson/missing
	$(MAKE) -C $@ && $(MAKE) $@-stage

jansson-install: jansson
	( cd $(STAGEDIR)/usr/lib && \
		install -d $(INSTALLDIR)/jansson/usr/lib && \
		rsync -avcH libjansson.so* $(INSTALLDIR)/jansson/usr/lib && \
		$(STRIP) $(INSTALLDIR)/jansson/usr/lib/*.so \
	)

jansson-clean:
	[ ! -f jansson/Makefile ] || $(MAKE) -C jansson distclean
	@rm -f jansson/Makefile
