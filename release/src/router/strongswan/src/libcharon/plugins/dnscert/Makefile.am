AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-dnscert.la
else
plugin_LTLIBRARIES = libstrongswan-dnscert.la
endif

libstrongswan_dnscert_la_SOURCES = \
	dnscert_plugin.h dnscert_plugin.c \
	dnscert_cred.h dnscert_cred.c \
	dnscert.h dnscert.c

libstrongswan_dnscert_la_LDFLAGS = -module -avoid-version
