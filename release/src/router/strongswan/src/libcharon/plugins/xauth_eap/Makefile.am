AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-xauth-eap.la
else
plugin_LTLIBRARIES = libstrongswan-xauth-eap.la
endif

libstrongswan_xauth_eap_la_SOURCES = \
	xauth_eap_plugin.h xauth_eap_plugin.c \
	xauth_eap.h xauth_eap.c

libstrongswan_xauth_eap_la_LDFLAGS = -module -avoid-version
