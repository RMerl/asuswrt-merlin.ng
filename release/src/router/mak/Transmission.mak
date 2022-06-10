Transmission/build/Makefile:
	@[ -e Transmission/build ] || mkdir -p Transmission/build
	@cd Transmission/build && cmake \
		-DCMAKE_EXE_LINKER_FLAGS="$(CMAKE_EXE_LINKER_FLAGS) $(EXTRALDFLAGS)" \
		-DENABLE_TESTS=OFF \
		-DZLIB_LIBRARY=$(STAGEDIR)/usr/lib/libz.so \
		..

Transmission: curl-7.21.7 libevent-2.0.21 zlib Transmission/build/Makefile
	@$(SEP)
	$(MAKE) -C $@/build

Transmission-install: Transmission
	install -D $</build/daemon/transmission-daemon $(INSTALLDIR)/$</usr/sbin/transmission-daemon
	install -D $</build/utils/transmission-remote $(INSTALLDIR)/$</usr/sbin/transmission-remote
	$(STRIP) $(INSTALLDIR)/$</usr/sbin/*

Transmission-clean:
	@[ ! -e Transmission/build ] || rm -fr Transmission/build
