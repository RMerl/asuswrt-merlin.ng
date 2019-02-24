AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-test-vectors.la
else
plugin_LTLIBRARIES = libstrongswan-test-vectors.la
endif

libstrongswan_test_vectors_la_SOURCES = \
	test_vectors_plugin.h test_vectors_plugin.c test_vectors.h \
	test_vectors/3des_cbc.c \
	test_vectors/aes_cbc.c \
	test_vectors/aes_ctr.c \
	test_vectors/aes_xcbc.c \
	test_vectors/aes_cmac.c \
	test_vectors/aes_ccm.c \
	test_vectors/aes_gcm.c \
	test_vectors/chacha20poly1305.c \
	test_vectors/chacha20_xof.c \
	test_vectors/blowfish.c \
	test_vectors/camellia_cbc.c \
	test_vectors/camellia_ctr.c \
	test_vectors/camellia_xcbc.c \
	test_vectors/cast.c \
	test_vectors/des.c \
	test_vectors/idea.c \
	test_vectors/null.c \
	test_vectors/rc2.c \
	test_vectors/rc5.c \
	test_vectors/serpent_cbc.c \
	test_vectors/twofish_cbc.c \
	test_vectors/md2.c \
	test_vectors/md4.c \
	test_vectors/md5.c \
	test_vectors/md5_hmac.c \
	test_vectors/sha1.c \
	test_vectors/sha1_hmac.c \
	test_vectors/sha2.c \
	test_vectors/sha2_hmac.c \
	test_vectors/sha3.c \
	test_vectors/sha3_shake.c \
	test_vectors/fips_prf.c \
	test_vectors/modp.c \
	test_vectors/modpsub.c \
	test_vectors/ecp.c \
	test_vectors/ecpbp.c \
	test_vectors/curve25519.c \
	test_vectors/curve448.c \
	test_vectors/rng.c

libstrongswan_test_vectors_la_LDFLAGS = -module -avoid-version
