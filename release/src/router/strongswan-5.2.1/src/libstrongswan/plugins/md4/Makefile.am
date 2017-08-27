AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-md4.la
else
plugin_LTLIBRARIES = libstrongswan-md4.la
endif

libstrongswan_md4_la_SOURCES = \
	md4_plugin.h md4_plugin.c md4_hasher.c md4_hasher.h

libstrongswan_md4_la_LDFLAGS = -module -avoid-version
