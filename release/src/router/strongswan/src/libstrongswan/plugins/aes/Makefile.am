AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-aes.la
else
plugin_LTLIBRARIES = libstrongswan-aes.la
endif

libstrongswan_aes_la_SOURCES = \
	aes_plugin.h aes_plugin.c aes_crypter.c aes_crypter.h

libstrongswan_aes_la_LDFLAGS = -module -avoid-version
