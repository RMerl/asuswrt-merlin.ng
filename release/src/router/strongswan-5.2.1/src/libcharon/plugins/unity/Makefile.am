AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-unity.la
else
plugin_LTLIBRARIES = libstrongswan-unity.la
endif

libstrongswan_unity_la_SOURCES = \
	unity_plugin.h unity_plugin.c \
	unity_handler.h unity_handler.c \
	unity_narrow.h unity_narrow.c \
	unity_provider.h unity_provider.c

libstrongswan_unity_la_LDFLAGS = -module -avoid-version
