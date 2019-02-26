AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libsimaka

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-aka.la
else
plugin_LTLIBRARIES = libstrongswan-eap-aka.la
libstrongswan_eap_aka_la_LIBADD = $(top_builddir)/src/libsimaka/libsimaka.la
endif

libstrongswan_eap_aka_la_SOURCES = \
	eap_aka_plugin.h eap_aka_plugin.c \
	eap_aka_peer.h eap_aka_peer.c \
	eap_aka_server.h eap_aka_server.c

libstrongswan_eap_aka_la_LDFLAGS = -module -avoid-version
