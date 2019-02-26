AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-dynamic.la
else
plugin_LTLIBRARIES = libstrongswan-eap-dynamic.la
endif

libstrongswan_eap_dynamic_la_SOURCES = \
	eap_dynamic_plugin.h eap_dynamic_plugin.c eap_dynamic.h eap_dynamic.c

libstrongswan_eap_dynamic_la_LDFLAGS = -module -avoid-version
