AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libtls

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-tls.la
else
plugin_LTLIBRARIES = libstrongswan-eap-tls.la
libstrongswan_eap_tls_la_LIBADD = $(top_builddir)/src/libtls/libtls.la
endif

libstrongswan_eap_tls_la_SOURCES = \
	eap_tls_plugin.h eap_tls_plugin.c eap_tls.h eap_tls.c

libstrongswan_eap_tls_la_LDFLAGS = -module -avoid-version
