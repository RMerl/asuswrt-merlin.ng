DEBUG_VALGRIND:=

valgrind: valgrind/Makefile
	$(MAKE) -C valgrind -j8 all && $(MAKE) $@-stage

valgrind/Makefile:
	$(MAKE) valgrind-configure

valgrind-configure:
	( cd valgrind ; \
		$(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
		--libexecdir=/usr/lib \
		--enable-tls --without-x --without-mpicc --without-uiout --disable-valgrindmi \
		--disable-tui --disable-valgrindtk --without-included-gettext --with-pagesize=4 \
		CFLAGS="$(if $(DEBUG_VALGRIND),-g) -fno-stack-protector $(if $(HND_ROUTER),,-D__UCLIBC__) $(EXTRACFLAGS)" \
		LDFLAGS="$(if $(HND_ROUTER),,$(EXTRALDFLAGS))" \
	)

valgrind-install: valgrind
	$(if $(HND_ROUTER),,install -D $(STAGEDIR)/usr/sbin/valgrind $(INSTALLDIR)/valgrind/usr/sbin/valgrind)
	make -C valgrind -j8 DESTDIR=$(INSTALLDIR)/valgrind install
	@rm -fr $(INSTALLDIR)/valgrind/usr/{include,share}
	@rm -fr $(INSTALLDIR)/valgrind/usr/lib/valgrind/*.a
	$(if $(DEBUG_VALGRIND),,$(STRIP) $(INSTALLDIR)/valgrind/usr/sbin/{cg_merge,valgrind,valgrind-di-server,valgrind-listener,vgdb})
	$(if $(DEBUG_VALGRIND),,$(STRIP) $(INSTALLDIR)/valgrind/usr/lib/valgrind/{*-linux,*.so})

valgrind-clean:
	[ ! -f valgrind/Makefile ] || $(MAKE) -C valgrind clean
