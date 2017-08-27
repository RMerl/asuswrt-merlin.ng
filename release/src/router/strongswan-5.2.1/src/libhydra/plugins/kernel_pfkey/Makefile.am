AM_CPPFLAGS = \
	-I${linux_headers} \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-kernel-pfkey.la
else
plugin_LTLIBRARIES = libstrongswan-kernel-pfkey.la
endif

libstrongswan_kernel_pfkey_la_SOURCES = \
	kernel_pfkey_plugin.h kernel_pfkey_plugin.c \
	kernel_pfkey_ipsec.h kernel_pfkey_ipsec.c

libstrongswan_kernel_pfkey_la_LDFLAGS = -module -avoid-version
