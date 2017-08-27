AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libradius

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-radattr.la
else
libstrongswan_radattr_la_LIBADD = $(top_builddir)/src/libradius/libradius.la
plugin_LTLIBRARIES = libstrongswan-radattr.la
endif

libstrongswan_radattr_la_SOURCES = radattr_plugin.h radattr_plugin.c \
	radattr_listener.h radattr_listener.c

libstrongswan_radattr_la_LDFLAGS = -module -avoid-version
