AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-dnskey.la
else
plugin_LTLIBRARIES = libstrongswan-dnskey.la
endif

libstrongswan_dnskey_la_SOURCES = \
	dnskey_plugin.h dnskey_plugin.c \
	dnskey_builder.h dnskey_builder.c \
	dnskey_encoder.h dnskey_encoder.c

libstrongswan_dnskey_la_LDFLAGS = -module -avoid-version
