AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-rc2.la
else
plugin_LTLIBRARIES = libstrongswan-rc2.la
endif

libstrongswan_rc2_la_SOURCES = \
	rc2_plugin.h rc2_plugin.c rc2_crypter.c rc2_crypter.h

libstrongswan_rc2_la_LDFLAGS = -module -avoid-version
