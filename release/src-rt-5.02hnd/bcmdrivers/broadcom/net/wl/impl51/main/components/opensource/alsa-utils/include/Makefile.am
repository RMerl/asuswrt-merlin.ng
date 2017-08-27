noinst_HEADERS=version.h gettext.h gettext_curses.h

version.h: stamp-vh
	@:

stamp-vh: $(top_builddir)/configure.in
	@echo "/*" > ver.tmp
	@echo " *  version.h" >> ver.tmp
	@echo " */" >> ver.tmp
	@echo "" >> ver.tmp
	@echo "#define SND_UTIL_MAJOR		$(SND_UTIL_MAJOR)" >> ver.tmp
	@echo "#define SND_UTIL_MINOR		$(SND_UTIL_MINOR)" >> ver.tmp
	@echo "#define SND_UTIL_SUBMINOR	$(SND_UTIL_SUBMINOR)" >> ver.tmp
	@echo "#define SND_UTIL_VERSION		((SND_UTIL_MAJOR<<16)|\\" >> ver.tmp
	@echo "				 (SND_UTIL_MINOR<<8)|\\" >> ver.tmp
	@echo "				  SND_UTIL_SUBMINOR)" >> ver.tmp
	@echo "#define SND_UTIL_VERSION_STR	\"$(SND_UTIL_VERSION)\"" >> ver.tmp
	@echo >> ver.tmp
	@cmp -s version.h ver.tmp \
          || (echo "Updating version.h"; \
              cp ver.tmp version.h; \
              echo timestamp > stamp-vh)
	-@rm -f ver.tmp

INCLUDES=-I$(top_srcdir)/include
