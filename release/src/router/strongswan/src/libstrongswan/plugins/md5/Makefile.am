AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-md5.la
else
plugin_LTLIBRARIES = libstrongswan-md5.la
endif

libstrongswan_md5_la_SOURCES = \
	md5_plugin.h md5_plugin.c md5_hasher.c md5_hasher.h

libstrongswan_md5_la_LDFLAGS = -module -avoid-version
