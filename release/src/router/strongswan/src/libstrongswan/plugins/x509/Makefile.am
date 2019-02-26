AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-x509.la
else
plugin_LTLIBRARIES = libstrongswan-x509.la
endif

libstrongswan_x509_la_SOURCES = \
	x509_plugin.h x509_plugin.c \
	x509_cert.h x509_cert.c \
	x509_crl.h x509_crl.c \
	x509_ac.h x509_ac.c \
	x509_pkcs10.h x509_pkcs10.c \
	x509_ocsp_request.h x509_ocsp_request.c \
	x509_ocsp_response.h x509_ocsp_response.c

libstrongswan_x509_la_LDFLAGS = -module -avoid-version
