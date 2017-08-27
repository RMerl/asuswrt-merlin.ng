AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-dhcp.la
else
plugin_LTLIBRARIES = libstrongswan-dhcp.la
endif

libstrongswan_dhcp_la_SOURCES = dhcp_plugin.h dhcp_plugin.c \
	dhcp_provider.h dhcp_provider.c \
	dhcp_socket.h dhcp_socket.c \
	dhcp_transaction.h dhcp_transaction.c

libstrongswan_dhcp_la_LDFLAGS = -module -avoid-version
