bonnie: bonnie/Makefile
	$(MAKE) CXX=$(CXX) -C bonnie

bonnie/Makefile:
	$(MAKE) bonnie-configure

bonnie-configure:
	( cd bonnie ; \
		$(CONFIGURE) \
		--prefix=/usr \
		--bindir=/usr/sbin \
		--libdir=/usr/lib \
	)

bonnie-install:
	install -D bonnie/bonnie++ $(INSTALLDIR)/bonnie/sbin/bonnie++
	$(STRIP) $(INSTALLDIR)/bonnie/sbin/bonnie++
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(HND_ROUTER),y)
	install -D $(TOOLCHAIN)/arm-buildroot-linux-gnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
else
	install -D $(shell dirname $(shell which $(CXX)))/../arm-brcm-linux-uclibcgnueabi/lib/libstdc++.so.6 $(INSTALLDIR)/ftpclient/usr/lib/libstdc++.so.6
endif
else
	install -D $(shell dirname $(shell which $(CXX)))/../lib/libstdc++.so.6 $(INSTALLDIR)/bonnie/usr/lib/libstdc++.so.6
endif

bonnie-clean:
	[ ! -f bonnie/Makefile ] || $(MAKE) -C bonnie clean
	@rm -f bonnie/Makefile
