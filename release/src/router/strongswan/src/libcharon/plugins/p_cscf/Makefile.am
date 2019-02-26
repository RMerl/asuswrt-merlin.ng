AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-p-cscf.la
else
plugin_LTLIBRARIES = libstrongswan-p-cscf.la
endif

libstrongswan_p_cscf_la_SOURCES = \
	p_cscf_plugin.c p_cscf_plugin.h \
	p_cscf_handler.c p_cscf_handler.h

libstrongswan_p_cscf_la_LDFLAGS = -module -avoid-version
