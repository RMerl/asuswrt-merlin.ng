iperf3: iperf3/Makefile
	@$(SEP)
	$(MAKE) -C $@

iperf3/Makefile:
	# libstdc++.so.6 is required if you want to remove CFLAGS=-static below.
	( cd iperf3 ; $(if $(QCA),,CFLAGS=-static) $(CONFIGURE) \
		ac_cv_func_malloc_0_nonnull=yes $(if $(QCA),ac_cv_func_gettimeofday=yes ac_cv_func_inet_ntop=yes) \
		--prefix=/usr --bindir=/usr/bin --libdir=/usr/lib \
		--disable-static --disable-shared \
	)

iperf3-install:
	$(MAKE) -C iperf3 DESTDIR=$(INSTALLDIR)/iperf3 install
	$(RM) -fr $(INSTALLDIR)/usr/include $(INSTALLDIR)/iperf3/usr/share $(INSTALLDIR)/iperf3/usr/lib/*.la $(INSTALLDIR)/iperf3/usr/lib/*.a
ifeq ($(QCA),y)
	install -D $(TOOLCHAIN)/lib/libstdc++.so.6 $(INSTALLDIR)/iperf3/usr/lib/libstdc++.so.6
endif
	$(STRIP) $(INSTALLDIR)/iperf3/usr/bin/iperf3

iperf3-clean:
	[ ! -f iperf3/Makefile ] || $(MAKE) -C iperf3 distclean
	@rm -f iperf3/Makefile
