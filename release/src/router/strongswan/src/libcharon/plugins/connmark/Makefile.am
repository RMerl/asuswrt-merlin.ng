AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) $(libiptc_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-connmark.la
else
plugin_LTLIBRARIES = libstrongswan-connmark.la
endif

libstrongswan_connmark_la_SOURCES = \
	connmark_listener.h connmark_listener.c \
	connmark_plugin.h connmark_plugin.c

libstrongswan_connmark_la_LDFLAGS = -module -avoid-version
libstrongswan_connmark_la_LIBADD = $(libiptc_LIBS)
