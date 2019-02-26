AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtpmtss \
	-I$(top_srcdir)/src/libimcv

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) $(json_CFLAGS)

imcv_LTLIBRARIES = imv-swima.la

imv_swima_la_LIBADD = \
	$(top_builddir)/src/libimcv/libimcv.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(json_LIBS)

imv_swima_la_SOURCES = \
	imv_swima.c imv_swima_state.h imv_swima_state.c \
	imv_swima_agent.h imv_swima_agent.c

imv_swima_la_LDFLAGS = -module -avoid-version -no-undefined
