AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libipsec

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-kernel-libipsec.la
else
plugin_LTLIBRARIES = libstrongswan-kernel-libipsec.la
endif

libstrongswan_kernel_libipsec_la_SOURCES = \
	kernel_libipsec_plugin.h kernel_libipsec_plugin.c \
	kernel_libipsec_ipsec.h kernel_libipsec_ipsec.c \
	kernel_libipsec_router.h kernel_libipsec_router.c

libstrongswan_kernel_libipsec_la_LIBADD = $(top_builddir)/src/libipsec/libipsec.la

libstrongswan_kernel_libipsec_la_LDFLAGS = -module -avoid-version
