AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-pgp.la
else
plugin_LTLIBRARIES = libstrongswan-pgp.la
endif

libstrongswan_pgp_la_SOURCES = \
	pgp_plugin.h pgp_plugin.c \
	pgp_utils.h pgp_utils.c \
	pgp_cert.h pgp_cert.c \
	pgp_encoder.h pgp_encoder.c \
	pgp_builder.h pgp_builder.c

libstrongswan_pgp_la_LDFLAGS = -module -avoid-version
