AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-gcrypt.la
else
plugin_LTLIBRARIES = libstrongswan-gcrypt.la
endif

libstrongswan_gcrypt_la_SOURCES = \
	gcrypt_plugin.h gcrypt_plugin.c \
	gcrypt_rsa_public_key.h gcrypt_rsa_public_key.c \
	gcrypt_rsa_private_key.h gcrypt_rsa_private_key.c \
	gcrypt_dh.h gcrypt_dh.c \
	gcrypt_rng.h gcrypt_rng.c \
	gcrypt_crypter.h gcrypt_crypter.c \
	gcrypt_hasher.h gcrypt_hasher.c

libstrongswan_gcrypt_la_LDFLAGS = -module -avoid-version
libstrongswan_gcrypt_la_LIBADD  = -lgcrypt -lgpg-error
