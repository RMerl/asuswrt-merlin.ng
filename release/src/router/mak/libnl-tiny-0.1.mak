libnl-tiny-0.1:
	$(MAKE) -C $@ && $(MAKE) $@-stage

libnl-tiny-0.1-clean:
	$(MAKE) -C libnl-tiny-0.1 clean

libnl-tiny-0.1-install: libnl-tiny-0.1
	install -D libnl-tiny-0.1/libnl-tiny.so $(INSTALLDIR)/libnl-tiny-0.1/usr/lib/libnl-tiny.so
	$(STRIP) $(INSTALLDIR)/libnl-tiny-0.1/usr/lib/libnl-tiny.so

libnl-tiny-0.1-stage:
	$(MAKE) -C libnl-tiny-0.1 stage
