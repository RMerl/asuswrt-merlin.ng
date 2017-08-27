AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-fips-prf.la
else
plugin_LTLIBRARIES = libstrongswan-fips-prf.la
endif

libstrongswan_fips_prf_la_SOURCES = \
	fips_prf_plugin.h fips_prf_plugin.c fips_prf.c fips_prf.h

libstrongswan_fips_prf_la_LDFLAGS = -module -avoid-version
