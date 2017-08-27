AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-des.la
else
plugin_LTLIBRARIES = libstrongswan-des.la
endif

libstrongswan_des_la_SOURCES = \
	des_plugin.h des_plugin.c des_crypter.c des_crypter.h

libstrongswan_des_la_LDFLAGS = -module -avoid-version
