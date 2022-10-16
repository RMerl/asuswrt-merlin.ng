AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS) \
	$(wolfssl_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-wolfssl.la
else
plugin_LTLIBRARIES = libstrongswan-wolfssl.la
endif

libstrongswan_wolfssl_la_SOURCES = \
	wolfssl_common.h \
	wolfssl_plugin.h wolfssl_plugin.c \
	wolfssl_aead.h wolfssl_aead.c \
	wolfssl_crypter.h wolfssl_crypter.c \
	wolfssl_diffie_hellman.h wolfssl_diffie_hellman.c \
	wolfssl_ec_diffie_hellman.h wolfssl_ec_diffie_hellman.c \
	wolfssl_ec_private_key.h wolfssl_ec_private_key.c \
	wolfssl_ec_public_key.h wolfssl_ec_public_key.c \
	wolfssl_ed_private_key.h wolfssl_ed_private_key.c \
	wolfssl_ed_public_key.h wolfssl_ed_public_key.c \
	wolfssl_hasher.h wolfssl_hasher.c \
	wolfssl_hmac.h wolfssl_hmac.c \
	wolfssl_kdf.h wolfssl_kdf.c \
	wolfssl_rsa_public_key.h wolfssl_rsa_public_key.c \
	wolfssl_rsa_private_key.h wolfssl_rsa_private_key.c \
	wolfssl_rng.h wolfssl_rng.c \
	wolfssl_sha1_prf.h wolfssl_sha1_prf.c \
	wolfssl_x_diffie_hellman.h wolfssl_x_diffie_hellman.c \
	wolfssl_util.h wolfssl_util.c \
	wolfssl_xof.h wolfssl_xof.c


libstrongswan_wolfssl_la_LDFLAGS = -module -avoid-version
libstrongswan_wolfssl_la_LIBADD  = $(wolfssl_LIBS)
