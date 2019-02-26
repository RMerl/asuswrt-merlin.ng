AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) \
	$(botan_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-botan.la
else
plugin_LTLIBRARIES = libstrongswan-botan.la
endif

libstrongswan_botan_la_SOURCES = \
	botan_plugin.h botan_plugin.c \
	botan_rng.h botan_rng.c \
	botan_hasher.h botan_hasher.c \
	botan_hmac.h botan_hmac.c \
	botan_crypter.h botan_crypter.c \
	botan_rsa_public_key.h botan_rsa_public_key.c \
	botan_rsa_private_key.h botan_rsa_private_key.c \
	botan_diffie_hellman.h botan_diffie_hellman.c \
	botan_ec_diffie_hellman.h botan_ec_diffie_hellman.c \
	botan_ec_public_key.h botan_ec_public_key.c \
	botan_ec_private_key.h botan_ec_private_key.c \
	botan_ed_public_key.h botan_ed_public_key.c \
	botan_ed_private_key.h botan_ed_private_key.c \
	botan_util.h botan_util.c \
	botan_util_keys.h botan_util_keys.c \
	botan_aead.h botan_aead.c \
	botan_x25519.h botan_x25519.c

libstrongswan_botan_la_LDFLAGS = -module -avoid-version
libstrongswan_botan_la_LIBADD = $(botan_LIBS)
