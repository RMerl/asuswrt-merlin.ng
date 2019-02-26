AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libtls

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-peap.la
else
plugin_LTLIBRARIES = libstrongswan-eap-peap.la
libstrongswan_eap_peap_la_LIBADD = $(top_builddir)/src/libtls/libtls.la
endif

libstrongswan_eap_peap_la_SOURCES = \
	eap_peap_plugin.h eap_peap_plugin.c \
	eap_peap.h eap_peap.c \
	eap_peap_peer.h eap_peap_peer.c \
	eap_peap_server.h eap_peap_server.c \
	eap_peap_avp.h eap_peap_avp.c

libstrongswan_eap_peap_la_LDFLAGS = -module -avoid-version
