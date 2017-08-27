AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libradius \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtnccs \
	-I$(top_srcdir)/src/libtls \
	-I$(top_srcdir)/src/libpttls

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-tnc-pdp.la
else
plugin_LTLIBRARIES = libstrongswan-tnc-pdp.la
endif

libstrongswan_tnc_pdp_la_LIBADD = \
	$(top_builddir)/src/libradius/libradius.la \
	$(top_builddir)/src/libpttls/libpttls.la \
	$(top_builddir)/src/libtls/libtls.la \
	$(top_builddir)/src/libtnccs/libtnccs.la

libstrongswan_tnc_pdp_la_SOURCES = \
	tnc_pdp_plugin.h tnc_pdp_plugin.c \
	tnc_pdp.h tnc_pdp.c tnc_pdp_connections.h tnc_pdp_connections.c

libstrongswan_tnc_pdp_la_LDFLAGS = -module -avoid-version
