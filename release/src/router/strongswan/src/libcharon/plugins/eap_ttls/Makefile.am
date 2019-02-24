AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libtls \
	-I$(top_srcdir)/src/libradius

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-ttls.la
else
plugin_LTLIBRARIES = libstrongswan-eap-ttls.la
libstrongswan_eap_ttls_la_LIBADD = $(top_builddir)/src/libtls/libtls.la
endif

libstrongswan_eap_ttls_la_SOURCES = \
	eap_ttls_plugin.h eap_ttls_plugin.c \
	eap_ttls_avp.h eap_ttls_avp.c \
	eap_ttls.h eap_ttls.c \
	eap_ttls_peer.h eap_ttls_peer.c \
	eap_ttls_server.h eap_ttls_server.c

libstrongswan_eap_ttls_la_LDFLAGS = -module -avoid-version
