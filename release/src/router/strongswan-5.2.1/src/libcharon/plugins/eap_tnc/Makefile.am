AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libtls \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtnccs

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-tnc.la
else
plugin_LTLIBRARIES = libstrongswan-eap-tnc.la
libstrongswan_eap_tnc_la_LIBADD = \
	$(top_builddir)/src/libtls/libtls.la \
	$(top_builddir)/src/libtnccs/libtnccs.la
endif

libstrongswan_eap_tnc_la_SOURCES = \
	eap_tnc_plugin.h eap_tnc_plugin.c eap_tnc.h eap_tnc.c

libstrongswan_eap_tnc_la_LDFLAGS = -module -avoid-version
