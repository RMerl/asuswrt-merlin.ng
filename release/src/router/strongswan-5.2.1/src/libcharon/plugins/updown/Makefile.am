AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-updown.la
else
plugin_LTLIBRARIES = libstrongswan-updown.la
endif

libstrongswan_updown_la_SOURCES = \
	updown_plugin.h updown_plugin.c \
	updown_handler.h updown_handler.c \
	updown_listener.h updown_listener.c

libstrongswan_updown_la_LDFLAGS = -module -avoid-version
