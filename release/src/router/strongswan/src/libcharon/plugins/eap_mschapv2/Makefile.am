AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-mschapv2.la
else
plugin_LTLIBRARIES = libstrongswan-eap-mschapv2.la
endif

libstrongswan_eap_mschapv2_la_SOURCES = \
	eap_mschapv2_plugin.h eap_mschapv2_plugin.c \
	eap_mschapv2.h eap_mschapv2.c

libstrongswan_eap_mschapv2_la_LDFLAGS = -module -avoid-version
