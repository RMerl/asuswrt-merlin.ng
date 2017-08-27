AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-pkcs7.la
else
plugin_LTLIBRARIES = libstrongswan-pkcs7.la
endif

libstrongswan_pkcs7_la_SOURCES = \
	pkcs7_generic.h pkcs7_generic.c \
	pkcs7_signed_data.h pkcs7_signed_data.c \
	pkcs7_encrypted_data.h pkcs7_encrypted_data.c \
	pkcs7_enveloped_data.h pkcs7_enveloped_data.c \
	pkcs7_data.h pkcs7_data.c \
	pkcs7_attributes.h pkcs7_attributes.c \
	pkcs7_plugin.h pkcs7_plugin.c

libstrongswan_pkcs7_la_LDFLAGS = -module -avoid-version
