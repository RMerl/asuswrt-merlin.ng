AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-ctr.la
else
plugin_LTLIBRARIES = libstrongswan-ctr.la
endif

libstrongswan_ctr_la_SOURCES = \
	ctr_plugin.h ctr_plugin.c \
	ctr_ipsec_crypter.h ctr_ipsec_crypter.c

libstrongswan_ctr_la_LDFLAGS = -module -avoid-version
