bin_PROGRAMS = aikgen

aikgen_SOURCES = aikgen.c

aikgen_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la
aikgen.o :	$(top_builddir)/config.status

if USE_TROUSERS
  aikgen_LDADD += -ltspi
endif

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-DIPSEC_CONFDIR=\"${sysconfdir}\" \
	-DPLUGINS=\""${aikgen_plugins}\""
