AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtpmtss \
	-I$(top_srcdir)/src/libimcv

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

imcv_LTLIBRARIES = imv-hcd.la

imv_hcd_la_LIBADD = $(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

imv_hcd_la_SOURCES = \
	imv_hcd.c imv_hcd_state.h imv_hcd_state.c \
	imv_hcd_agent.h imv_hcd_agent.c

imv_hcd_la_LDFLAGS = -module -avoid-version -no-undefined
