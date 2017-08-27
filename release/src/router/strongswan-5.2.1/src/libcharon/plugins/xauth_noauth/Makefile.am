AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-xauth-noauth.la
else
plugin_LTLIBRARIES = libstrongswan-xauth-noauth.la
endif

libstrongswan_xauth_noauth_la_SOURCES = \
	xauth_noauth_plugin.h xauth_noauth_plugin.c \
	xauth_noauth.h xauth_noauth.c

libstrongswan_xauth_noauth_la_LDFLAGS = -module -avoid-version
