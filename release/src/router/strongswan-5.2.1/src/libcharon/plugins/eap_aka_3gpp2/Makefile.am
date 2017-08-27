AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libsimaka

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

libstrongswan_eap_aka_3gpp2_la_LDFLAGS = -module -avoid-version
libstrongswan_eap_aka_3gpp2_la_LIBADD = -lgmp

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-aka-3gpp2.la
else
plugin_LTLIBRARIES = libstrongswan-eap-aka-3gpp2.la
libstrongswan_eap_aka_3gpp2_la_LIBADD += $(top_builddir)/src/libsimaka/libsimaka.la
endif

libstrongswan_eap_aka_3gpp2_la_SOURCES = \
	eap_aka_3gpp2_plugin.h eap_aka_3gpp2_plugin.c \
	eap_aka_3gpp2_card.h eap_aka_3gpp2_card.c \
	eap_aka_3gpp2_provider.h eap_aka_3gpp2_provider.c \
	eap_aka_3gpp2_functions.h eap_aka_3gpp2_functions.c
