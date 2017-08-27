AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-curl.la
else
plugin_LTLIBRARIES = libstrongswan-curl.la
endif

libstrongswan_curl_la_SOURCES = \
	curl_plugin.h curl_plugin.c curl_fetcher.c curl_fetcher.h

libstrongswan_curl_la_LDFLAGS = -module -avoid-version
libstrongswan_curl_la_LIBADD  = -lcurl
