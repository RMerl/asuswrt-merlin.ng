AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-xauth-generic.la
else
plugin_LTLIBRARIES = libstrongswan-xauth-generic.la
endif

libstrongswan_xauth_generic_la_SOURCES = \
	xauth_generic_plugin.h xauth_generic_plugin.c \
	xauth_generic.h xauth_generic.c

libstrongswan_xauth_generic_la_LDFLAGS = -module -avoid-version
