AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-pkcs1.la
else
plugin_LTLIBRARIES = libstrongswan-pkcs1.la
endif

libstrongswan_pkcs1_la_SOURCES = \
	pkcs1_plugin.h pkcs1_plugin.c \
	pkcs1_encoder.h pkcs1_encoder.c \
	pkcs1_builder.h pkcs1_builder.c

libstrongswan_pkcs1_la_LDFLAGS = -module -avoid-version
