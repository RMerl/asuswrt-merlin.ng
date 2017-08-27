AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS =  \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-identity.la
else
plugin_LTLIBRARIES = libstrongswan-eap-identity.la
endif

libstrongswan_eap_identity_la_SOURCES = \
	eap_identity_plugin.h eap_identity_plugin.c eap_identity.h eap_identity.c

libstrongswan_eap_identity_la_LDFLAGS = -module -avoid-version
