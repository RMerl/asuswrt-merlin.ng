EXTRA_DIST = ext/dumm.c ext/README \
  ext/lib/dumm.rb ext/lib/dumm/guest.rb

ipseclib_LTLIBRARIES = libdumm.la
ipsec_PROGRAMS = dumm irdumm

libdumm_la_SOURCES = dumm.c dumm.h guest.c guest.h iface.c iface.h \
  bridge.c bridge.h mconsole.c mconsole.h cowfs.h cowfs.c
dumm_SOURCES = main.c
irdumm_SOURCES = irdumm.c

libdumm_la_LIBADD = -lbridge -lfuse -lutil $(top_builddir)/src/libstrongswan/libstrongswan.la
dumm_LDADD = libdumm.la ${gtk_LIBS} $(top_builddir)/src/libstrongswan/libstrongswan.la
irdumm_LDADD = libdumm.la ${RUBYLIB} $(top_builddir)/src/libstrongswan/libstrongswan.la

AM_CPPFLAGS = \
	-D_FILE_OFFSET_BITS=64 \
	-I$(top_srcdir)/src/libstrongswan \
	${RUBYINCLUDE}

AM_CFLAGS = \
	${gtk_CFLAGS}

all-local: ext

clean-local:
	(test -f ext/Makefile && cd ext && $(MAKE) clean && rm Makefile || true)

install-data-local:
	(test -f ext/Makefile && cd ext && $(MAKE) install)

ext:	libdumm.la
	(cd ext && $(RUBY) extconf.rb && $(MAKE))

.PHONY: ext
