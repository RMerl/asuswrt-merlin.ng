AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtls \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtnccs

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-tnccs-dynamic.la
else
plugin_LTLIBRARIES = libstrongswan-tnccs-dynamic.la
libstrongswan_tnccs_dynamic_la_LIBADD = \
	$(top_builddir)/src/libtncif/libtncif.la \
	$(top_builddir)/src/libtnccs/libtnccs.la
endif

libstrongswan_tnccs_dynamic_la_SOURCES = \
	tnccs_dynamic_plugin.h tnccs_dynamic_plugin.c tnccs_dynamic.h tnccs_dynamic.c

libstrongswan_tnccs_dynamic_la_LDFLAGS = -module -avoid-version
