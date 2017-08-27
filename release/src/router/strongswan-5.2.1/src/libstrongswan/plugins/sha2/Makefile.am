AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-sha2.la
else
plugin_LTLIBRARIES = libstrongswan-sha2.la
endif

libstrongswan_sha2_la_SOURCES = \
	sha2_plugin.h sha2_plugin.c sha2_hasher.c sha2_hasher.h

libstrongswan_sha2_la_LDFLAGS = -module -avoid-version
