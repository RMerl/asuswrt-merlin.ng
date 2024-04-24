AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-openxpki.la
else
plugin_LTLIBRARIES = libstrongswan-openxpki.la
endif

libstrongswan_openxpki_la_SOURCES = \
	openxpki_plugin.h openxpki_plugin.c \
	openxpki_ocsp_responder.h openxpki_ocsp_responder.c

libstrongswan_openxpki_la_LDFLAGS = -module -avoid-version
