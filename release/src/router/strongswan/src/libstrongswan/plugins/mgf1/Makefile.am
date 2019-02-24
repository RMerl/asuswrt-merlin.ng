AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-mgf1.la
else
plugin_LTLIBRARIES = libstrongswan-mgf1.la
endif

libstrongswan_mgf1_la_SOURCES = \
	mgf1_plugin.h mgf1_plugin.c \
	mgf1_xof.h mgf1_xof.c

libstrongswan_mgf1_la_LDFLAGS = -module -avoid-version
