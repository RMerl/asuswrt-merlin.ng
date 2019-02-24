AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-bypass-lan.la
else
plugin_LTLIBRARIES = libstrongswan-bypass-lan.la
endif

libstrongswan_bypass_lan_la_SOURCES = \
	bypass_lan_plugin.h bypass_lan_plugin.c \
	bypass_lan_listener.h bypass_lan_listener.c

libstrongswan_bypass_lan_la_LDFLAGS = -module -avoid-version
