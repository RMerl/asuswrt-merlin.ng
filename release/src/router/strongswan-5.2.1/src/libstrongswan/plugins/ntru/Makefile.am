AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) \
	@COVERAGE_CFLAGS@

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-ntru.la
else
plugin_LTLIBRARIES = libstrongswan-ntru.la
endif

libstrongswan_ntru_la_SOURCES = \
	ntru_plugin.h ntru_plugin.c \
	ntru_convert.h ntru_convert.c \
	ntru_drbg.h ntru_drbg.c \
	ntru_ke.h ntru_ke.c \
	ntru_mgf1.h ntru_mgf1.c \
	ntru_param_set.h ntru_param_set.c \
	ntru_poly.h ntru_poly.c \
	ntru_public_key.h ntru_public_key.c \
	ntru_private_key.h ntru_private_key.c \
	ntru_trits.h ntru_trits.c

libstrongswan_ntru_la_LDFLAGS = -module -avoid-version
