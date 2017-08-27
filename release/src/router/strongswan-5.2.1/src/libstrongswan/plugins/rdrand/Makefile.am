AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-rdrand.la
else
plugin_LTLIBRARIES = libstrongswan-rdrand.la
endif

libstrongswan_rdrand_la_SOURCES = \
	rdrand_plugin.h rdrand_plugin.c \
	rdrand_rng.h rdrand_rng.c

libstrongswan_rdrand_la_LDFLAGS = -module -avoid-version
