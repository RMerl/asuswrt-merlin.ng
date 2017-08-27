AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-xcbc.la
else
plugin_LTLIBRARIES = libstrongswan-xcbc.la
endif

libstrongswan_xcbc_la_SOURCES = \
	xcbc_plugin.h xcbc_plugin.c xcbc.h xcbc.c

libstrongswan_xcbc_la_LDFLAGS = -module -avoid-version
