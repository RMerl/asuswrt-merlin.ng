if SHIP_BINARIES

EXTRA_DIST = include bin

SH_DIST_HOOK = dist-ship-binary-hook
dist-ship-binary-hook include bin:
	cd "$(top_builddir)" && $(MAKE) includedir="$(PWD)/include" DESTDIR="" prefix="$(PWD)/tmp" install
	$(MKDIR_P) "$(PWD)/bin"
	cp "$(PWD)/tmp/bin/"*.dll "bin/"

endif

dist-hook: $(SH_DIST_HOOK)
