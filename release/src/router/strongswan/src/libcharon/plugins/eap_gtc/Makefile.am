AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-gtc.la
else
plugin_LTLIBRARIES = libstrongswan-eap-gtc.la
endif

libstrongswan_eap_gtc_la_SOURCES = \
	eap_gtc_plugin.h eap_gtc_plugin.c eap_gtc.h eap_gtc.c

libstrongswan_eap_gtc_la_LDFLAGS = -module -avoid-version
