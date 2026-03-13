ifneq (,$(filter $(ASUSWRT_BRCM_SDK_VERSION),WIFI7_SDK_20250506 WIFI8_SDK_20251126))
PLATFORM_EXT_CMD := \
  if [ -d iperf3/dep_wifi70506 ]; then \
    if command -v libtoolize >/dev/null 2>&1; then \
      ltver="$$(libtoolize --version 2>/dev/null | head -n1)"; \
      if [ -n "$$ltver" ]; then \
        case "$$ltver" in \
          *" 2.4.6"*) cp -rf iperf3/dep_wifi70506/* iperf3/ ;; \
        esac; \
      fi; \
    else \
      echo "libtoolize not found; skipping iperf3/dep_wifi70506 patches"; \
    fi; \
  fi
else
PLATFORM_EXT_CMD := "echo \"skip\""
endif

iperf3: iperf3/Makefile
	@$(SEP)
	$(MAKE) -j8 -C $@

iperf3/Makefile: iperf3/configure
	# libstdc++.so.6 is required if you want to remove CFLAGS=-static below.
	( $(PLATFORM_EXT_CMD) ; cd iperf3 ; unset CPP; CFLAGS="-D_GNU_SOURCE $(if $(QCA),,-static)" $(CONFIGURE) \
		ac_cv_func_malloc_0_nonnull=yes $(if $(QCA),ac_cv_func_gettimeofday=yes ac_cv_func_inet_ntop=yes) \
		--prefix=/usr --bindir=/usr/bin --libdir=/usr/lib \
	)

iperf3/configure:
	( cd iperf3 ; ./bootstrap.sh )

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
