AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-sha1.la
else
plugin_LTLIBRARIES = libstrongswan-sha1.la
endif

libstrongswan_sha1_la_SOURCES = \
	sha1_plugin.h sha1_plugin.c \
	sha1_hasher.c sha1_hasher.h sha1_prf.c sha1_prf.h

libstrongswan_sha1_la_LDFLAGS = -module -avoid-version
