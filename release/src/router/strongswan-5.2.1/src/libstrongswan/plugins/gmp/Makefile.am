AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-gmp.la
else
plugin_LTLIBRARIES = libstrongswan-gmp.la
endif

libstrongswan_gmp_la_SOURCES = \
	gmp_plugin.h gmp_plugin.c \
	gmp_diffie_hellman.c gmp_diffie_hellman.h \
	gmp_rsa_private_key.c gmp_rsa_private_key.h \
	gmp_rsa_public_key.c gmp_rsa_public_key.h

libstrongswan_gmp_la_LDFLAGS = -module -avoid-version
libstrongswan_gmp_la_LIBADD  = -lgmp
