IPERF320_EXTRA_CFLAGS :=

ifneq (,$(filter $(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20250506 WIFI8_SDK_20251126 WIFI8_SDK_20260204 WIFI8_SDK_20260402))
IPERF320_EXTRA_CFLAGS += -DHAVE_TCP_DISCARD -fPIC -Wall -pthread -I.
endif

iperf-3.20-configure:
	cd iperf-3.20 && \
		unset CPP; \
		CFLAGS="$(IPERF320_EXTRA_CFLAGS)" \
		$(CONFIGURE) \
		ac_cv_func_malloc_0_nonnull=yes \
		$(if $(QCA),ac_cv_func_gettimeofday=yes ac_cv_func_inet_ntop=yes) \
		--prefix=/usr \
		--bindir=/usr/bin \
		--libdir=/usr/lib \
		--disable-shared \
		--enable-static

iperf-3.20/Makefile: Makefile iperf-3.20/configure
	$(MAKE) iperf-3.20-configure

iperf-3.20: iperf-3.20/Makefile
	@$(SEP)
	$(MAKE) -j8 -C $@

iperf-3.20-install: iperf-3.20
	$(MAKE) -C iperf-3.20 DESTDIR=$(INSTALLDIR)/iperf-3.20 install
	$(RM) -fr $(INSTALLDIR)/iperf-3.20/usr/include \
		$(INSTALLDIR)/iperf-3.20/usr/share \
		$(INSTALLDIR)/iperf-3.20/usr/lib/*.la \
		$(INSTALLDIR)/iperf-3.20/usr/lib/*.a
	$(STRIP) $(INSTALLDIR)/iperf-3.20/usr/bin/iperf3

iperf-3.20-clean:
	[ ! -f iperf-3.20/Makefile ] || $(MAKE) -C iperf-3.20 distclean
	@rm -f iperf-3.20/Makefile
