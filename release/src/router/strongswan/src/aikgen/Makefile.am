bin_PROGRAMS = aikgen

aikgen_SOURCES = aikgen.c

aikgen_LDADD = $(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libtpmtss/libtpmtss.la

aikgen.o :	$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtpmtss \
	-DIPSEC_CONFDIR=\"${sysconfdir}\" \
	-DPLUGINS=\""${aikgen_plugins}\""
