AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-ccm.la
else
plugin_LTLIBRARIES = libstrongswan-ccm.la
endif

libstrongswan_ccm_la_SOURCES = \
	ccm_plugin.h ccm_plugin.c \
	ccm_aead.h ccm_aead.c

libstrongswan_ccm_la_LDFLAGS = -module -avoid-version
