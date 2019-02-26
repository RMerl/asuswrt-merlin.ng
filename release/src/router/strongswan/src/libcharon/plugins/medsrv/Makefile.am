AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-medsrv.la
else
plugin_LTLIBRARIES = libstrongswan-medsrv.la
endif

libstrongswan_medsrv_la_SOURCES = \
	medsrv_plugin.h medsrv_plugin.c \
	medsrv_creds.h medsrv_creds.c \
	medsrv_config.h medsrv_config.c

libstrongswan_medsrv_la_LDFLAGS = -module -avoid-version
