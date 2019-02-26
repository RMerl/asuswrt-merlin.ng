AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-farp.la
else
plugin_LTLIBRARIES = libstrongswan-farp.la
endif

libstrongswan_farp_la_SOURCES = farp_plugin.h farp_plugin.c \
	farp_listener.h farp_listener.c farp_spoofer.h farp_spoofer.c

libstrongswan_farp_la_LDFLAGS = -module -avoid-version
