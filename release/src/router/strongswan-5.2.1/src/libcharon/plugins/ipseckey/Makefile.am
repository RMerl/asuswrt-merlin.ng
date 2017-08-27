AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-ipseckey.la
else
plugin_LTLIBRARIES = libstrongswan-ipseckey.la
endif

libstrongswan_ipseckey_la_SOURCES = \
	ipseckey_plugin.h ipseckey_plugin.c \
	ipseckey_cred.h ipseckey_cred.c \
	ipseckey.h ipseckey.c

libstrongswan_ipseckey_la_LDFLAGS = -module -avoid-version
