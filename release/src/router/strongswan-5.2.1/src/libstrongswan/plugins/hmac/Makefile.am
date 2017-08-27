AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-hmac.la
else
plugin_LTLIBRARIES = libstrongswan-hmac.la
endif

libstrongswan_hmac_la_SOURCES = \
	hmac_plugin.h hmac_plugin.c hmac.h hmac.c

libstrongswan_hmac_la_LDFLAGS = -module -avoid-version
