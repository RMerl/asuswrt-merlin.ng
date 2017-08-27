AM_CPPFLAGS = \
	-I${linux_headers} \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-kernel-pfroute.la
else
plugin_LTLIBRARIES = libstrongswan-kernel-pfroute.la
endif

libstrongswan_kernel_pfroute_la_SOURCES = \
	kernel_pfroute_plugin.h kernel_pfroute_plugin.c \
	kernel_pfroute_net.h kernel_pfroute_net.c

libstrongswan_kernel_pfroute_la_LDFLAGS = -module -avoid-version
