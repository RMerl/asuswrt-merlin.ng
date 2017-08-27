AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-md5.la
else
plugin_LTLIBRARIES = libstrongswan-eap-md5.la
endif

libstrongswan_eap_md5_la_SOURCES = \
	eap_md5_plugin.h eap_md5_plugin.c eap_md5.h eap_md5.c

libstrongswan_eap_md5_la_LDFLAGS = -module -avoid-version
