AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-DFIPS_MODE=${fips_mode}

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-openssl.la
else
plugin_LTLIBRARIES = libstrongswan-openssl.la
endif

libstrongswan_openssl_la_SOURCES = \
	openssl_plugin.h openssl_plugin.c \
	openssl_util.c openssl_util.h \
	openssl_crypter.c openssl_crypter.h \
	openssl_hasher.c openssl_hasher.h \
	openssl_sha1_prf.c openssl_sha1_prf.h \
	openssl_diffie_hellman.c openssl_diffie_hellman.h \
	openssl_rsa_private_key.c openssl_rsa_private_key.h \
	openssl_rsa_public_key.c openssl_rsa_public_key.h \
	openssl_ec_diffie_hellman.c openssl_ec_diffie_hellman.h \
	openssl_ec_private_key.c openssl_ec_private_key.h \
	openssl_ec_public_key.c openssl_ec_public_key.h \
	openssl_x509.c openssl_x509.h \
	openssl_crl.c openssl_crl.h \
	openssl_pkcs7.c openssl_pkcs7.h \
	openssl_pkcs12.c openssl_pkcs12.h \
	openssl_rng.c openssl_rng.h \
	openssl_hmac.c openssl_hmac.h \
	openssl_aead.c openssl_aead.h \
	openssl_x_diffie_hellman.c openssl_x_diffie_hellman.h \
	openssl_ed_private_key.c openssl_ed_private_key.h \
	openssl_ed_public_key.c openssl_ed_public_key.h

libstrongswan_openssl_la_LDFLAGS = -module -avoid-version
libstrongswan_openssl_la_LIBADD  = $(OPENSSL_LIB)
