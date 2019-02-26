AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-pkcs12.la
else
plugin_LTLIBRARIES = libstrongswan-pkcs12.la
endif

libstrongswan_pkcs12_la_SOURCES = \
	pkcs12_plugin.h pkcs12_plugin.c \
	pkcs12_decode.h pkcs12_decode.c

libstrongswan_pkcs12_la_LDFLAGS = -module -avoid-version
