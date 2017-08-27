AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-uci.la
else
plugin_LTLIBRARIES = libstrongswan-uci.la
endif

libstrongswan_uci_la_SOURCES = \
	uci_plugin.h uci_plugin.c uci_parser.h uci_parser.c \
	uci_config.h uci_config.c uci_creds.h uci_creds.c \
	uci_control.h uci_control.c

libstrongswan_uci_la_LDFLAGS = -module -avoid-version
libstrongswan_uci_la_LIBADD  = -luci
