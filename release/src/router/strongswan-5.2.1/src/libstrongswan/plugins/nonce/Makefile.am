AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-nonce.la
else
plugin_LTLIBRARIES = libstrongswan-nonce.la
endif

libstrongswan_nonce_la_SOURCES = \
	nonce_plugin.h nonce_plugin.c \
	nonce_nonceg.c nonce_nonceg.h

libstrongswan_nonce_la_LDFLAGS = -module -avoid-version
