iperf: iperf/Makefile
	@$(SEP)
	$(MAKE) -j8 -C $@

iperf/Makefile: iperf/configure
	# libstdc++.so.6 is required if you want to remove CFLAGS=-static below.
	( cd iperf ; $(if $(QCA)$(LANTIQ),,CFLAGS=-static) $(CONFIGURE) ac_cv_func_malloc_0_nonnull=yes $(if $(QCA),ac_cv_func_gettimeofday=yes ac_cv_func_inet_ntop=yes))

iperf/configure:
	( cd iperf ; ./autogen.sh )

iperf-install:
	install -D iperf/src/iperf $(INSTALLDIR)/iperf/usr/bin/iperf
ifeq ($(QCA),y)
	install -D $(TOOLCHAIN)/lib/libstdc++.so.6 $(INSTALLDIR)/iperf/usr/lib/libstdc++.so.6
endif
	$(STRIP) $(INSTALLDIR)/iperf/usr/bin/iperf

iperf-clean:
	[ ! -f iperf/Makefile ] || $(MAKE) -C iperf distclean
	@rm -f iperf/Makefile
