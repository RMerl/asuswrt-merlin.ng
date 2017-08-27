AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libimcv

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

imcv_LTLIBRARIES = imv-scanner.la

imv_scanner_la_LIBADD = $(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

imv_scanner_la_SOURCES = \
	imv_scanner.c imv_scanner_state.h imv_scanner_state.c \
	imv_scanner_agent.h imv_scanner_agent.c

imv_scanner_la_LDFLAGS = -module -avoid-version -no-undefined
