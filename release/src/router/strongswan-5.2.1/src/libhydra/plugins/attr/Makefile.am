AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-attr.la
else
plugin_LTLIBRARIES = libstrongswan-attr.la
endif

libstrongswan_attr_la_SOURCES = \
	attr_plugin.h attr_plugin.c \
	attr_provider.h attr_provider.c

libstrongswan_attr_la_LDFLAGS = -module -avoid-version
