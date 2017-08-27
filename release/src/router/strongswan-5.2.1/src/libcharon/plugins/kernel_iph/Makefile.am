AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-kernel-iph.la
else
plugin_LTLIBRARIES = libstrongswan-kernel-iph.la
endif

libstrongswan_kernel_iph_la_SOURCES = \
	kernel_iph_plugin.h kernel_iph_plugin.c \
	kernel_iph_net.h kernel_iph_net.c

libstrongswan_kernel_iph_la_LDFLAGS = -module -avoid-version
libstrongswan_kernel_iph_la_LIBADD = -liphlpapi
