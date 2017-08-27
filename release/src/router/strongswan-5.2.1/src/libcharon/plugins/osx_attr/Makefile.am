AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-osx-attr.la
else
plugin_LTLIBRARIES = libstrongswan-osx-attr.la
endif

libstrongswan_osx_attr_la_SOURCES = \
	osx_attr_plugin.c osx_attr_plugin.h \
	osx_attr_handler.c osx_attr_handler.h

libstrongswan_osx_attr_la_LDFLAGS = -module -avoid-version \
	-framework SystemConfiguration -framework CoreFoundation
