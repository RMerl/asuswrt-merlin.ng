AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-drbg.la
else
plugin_LTLIBRARIES = libstrongswan-drbg.la
endif

libstrongswan_drbg_la_SOURCES = \
	drbg_plugin.h drbg_plugin.c \
	drbg_ctr.h drbg_ctr.c \
	drbg_hmac.h drbg_hmac.c

libstrongswan_drbg_la_LDFLAGS = -module -avoid-version
