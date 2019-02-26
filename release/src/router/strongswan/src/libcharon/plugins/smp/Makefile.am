AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-DIPSEC_PIDDIR=\"${piddir}\"

AM_CFLAGS = \
	${xml_CFLAGS} \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-smp.la
else
plugin_LTLIBRARIES = libstrongswan-smp.la
endif

libstrongswan_smp_la_SOURCES = \
	smp.h smp.c

libstrongswan_smp_la_LDFLAGS = -module -avoid-version
libstrongswan_smp_la_LIBADD  = ${xml_LIBS}
