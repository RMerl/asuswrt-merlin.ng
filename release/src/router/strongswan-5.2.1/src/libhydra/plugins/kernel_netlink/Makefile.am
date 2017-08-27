AM_CPPFLAGS = \
	-I${linux_headers} \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-DROUTING_TABLE=${routing_table} \
	-DROUTING_TABLE_PRIO=${routing_table_prio}

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-kernel-netlink.la
else
plugin_LTLIBRARIES = libstrongswan-kernel-netlink.la
endif

libstrongswan_kernel_netlink_la_SOURCES = \
	kernel_netlink_plugin.h kernel_netlink_plugin.c \
	kernel_netlink_ipsec.h kernel_netlink_ipsec.c \
	kernel_netlink_net.h kernel_netlink_net.c \
	kernel_netlink_shared.h kernel_netlink_shared.c

libstrongswan_kernel_netlink_la_LDFLAGS = -module -avoid-version
