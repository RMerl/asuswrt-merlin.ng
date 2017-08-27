AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libimcv

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

imcv_LTLIBRARIES = imv-test.la

imv_test_la_LIBADD = $(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la

imv_test_la_SOURCES = \
	imv_test.c imv_test_state.h imv_test_state.c \
	imv_test_agent.h imv_test_agent.c

imv_test_la_LDFLAGS = -module -avoid-version -no-undefined
