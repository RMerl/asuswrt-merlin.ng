AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-medcli.la
else
plugin_LTLIBRARIES = libstrongswan-medcli.la
endif

libstrongswan_medcli_la_SOURCES = \
	medcli_plugin.h medcli_plugin.c \
	medcli_creds.h medcli_creds.c \
	medcli_config.h medcli_config.c \
	medcli_listener.h medcli_listener.c

libstrongswan_medcli_la_LDFLAGS = -module -avoid-version
