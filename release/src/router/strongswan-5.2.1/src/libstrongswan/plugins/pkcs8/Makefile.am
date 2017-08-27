AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-pkcs8.la
else
plugin_LTLIBRARIES = libstrongswan-pkcs8.la
endif

libstrongswan_pkcs8_la_SOURCES = \
	pkcs8_plugin.h pkcs8_plugin.c \
	pkcs8_builder.h pkcs8_builder.c

libstrongswan_pkcs8_la_LDFLAGS = -module -avoid-version
