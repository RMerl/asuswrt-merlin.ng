AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libimcv

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

imcv_LTLIBRARIES = imc-hcd.la

imc_hcd_la_LIBADD = $(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

imc_hcd_la_SOURCES = imc_hcd.c imc_hcd_state.h imc_hcd_state.c

imc_hcd_la_LDFLAGS = -module -avoid-version -no-undefined
