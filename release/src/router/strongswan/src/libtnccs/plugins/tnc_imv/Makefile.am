AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtnccs \
	-I$(top_srcdir)/src/libtls

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-tnc-imv.la
else
plugin_LTLIBRARIES = libstrongswan-tnc-imv.la
libstrongswan_tnc_imv_la_LIBADD = \
	$(top_builddir)/src/libtncif/libtncif.la \
	$(top_builddir)/src/libtnccs/libtnccs.la
endif

libstrongswan_tnc_imv_la_SOURCES = \
	tnc_imv_plugin.h tnc_imv_plugin.c tnc_imv.h tnc_imv.c \
	tnc_imv_manager.h tnc_imv_manager.c tnc_imv_bind_function.c \
	tnc_imv_recommendations.h tnc_imv_recommendations.c

libstrongswan_tnc_imv_la_LDFLAGS = -module -avoid-version
