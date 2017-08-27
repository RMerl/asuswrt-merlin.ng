AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtnccs \
	-I$(top_srcdir)/src/libtls

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-tnc-imc.la
else
plugin_LTLIBRARIES = libstrongswan-tnc-imc.la
libstrongswan_tnc_imc_la_LIBADD = \
	$(top_builddir)/src/libtncif/libtncif.la \
	$(top_builddir)/src/libtnccs/libtnccs.la
endif

libstrongswan_tnc_imc_la_SOURCES = \
	tnc_imc_plugin.h tnc_imc_plugin.c tnc_imc.h tnc_imc.c \
	tnc_imc_manager.h tnc_imc_manager.c tnc_imc_bind_function.c

libstrongswan_tnc_imc_la_LDFLAGS = -module -avoid-version
