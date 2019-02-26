AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-winhttp.la
else
plugin_LTLIBRARIES = libstrongswan-winhttp.la
endif

libstrongswan_winhttp_la_SOURCES = \
	winhttp_fetcher.c winhttp_fetcher.h \
	winhttp_plugin.c winhttp_plugin.h

libstrongswan_winhttp_la_LDFLAGS = -module -avoid-version
libstrongswan_winhttp_la_LIBADD  = -lwinhttp
