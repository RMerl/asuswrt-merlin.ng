ipseclib_LTLIBRARIES = libstrongswan.la

libstrongswan_la_SOURCES = \
library.c \
asn1/asn1.c asn1/asn1_parser.c asn1/oid.c bio/bio_reader.c bio/bio_writer.c \
collections/blocking_queue.c collections/enumerator.c collections/hashtable.c \
collections/array.c \
collections/linked_list.c crypto/crypters/crypter.c crypto/hashers/hasher.c \
crypto/proposal/proposal_keywords.c crypto/proposal/proposal_keywords_static.c \
crypto/prfs/prf.c crypto/prfs/mac_prf.c crypto/pkcs5.c \
crypto/rngs/rng.c crypto/prf_plus.c crypto/signers/signer.c \
crypto/signers/mac_signer.c crypto/crypto_factory.c crypto/crypto_tester.c \
crypto/diffie_hellman.c crypto/aead.c crypto/transform.c \
crypto/iv/iv_gen_rand.c crypto/iv/iv_gen_seq.c \
credentials/credential_factory.c credentials/builder.c \
credentials/cred_encoding.c credentials/keys/private_key.c \
credentials/keys/public_key.c credentials/keys/shared_key.c \
credentials/certificates/certificate.c credentials/certificates/crl.c \
credentials/certificates/ocsp_response.c \
credentials/containers/container.c credentials/containers/pkcs12.c \
credentials/credential_manager.c \
credentials/sets/auth_cfg_wrapper.c credentials/sets/ocsp_response_wrapper.c \
credentials/sets/cert_cache.c credentials/sets/mem_cred.c \
credentials/sets/callback_cred.c credentials/auth_cfg.c database/database.c \
database/database_factory.c fetcher/fetcher.c fetcher/fetcher_manager.c eap/eap.c \
ipsec/ipsec_types.c \
networking/host.c networking/host_resolver.c networking/packet.c \
networking/tun_device.c networking/streams/stream_manager.c \
networking/streams/stream.c networking/streams/stream_service.c \
networking/streams/stream_tcp.c networking/streams/stream_service_tcp.c \
pen/pen.c plugins/plugin_loader.c plugins/plugin_feature.c processing/jobs/job.c \
processing/jobs/callback_job.c processing/processor.c processing/scheduler.c \
processing/watcher.c resolver/resolver_manager.c resolver/rr_set.c \
selectors/traffic_selector.c settings/settings.c settings/settings_types.c \
settings/settings_parser.y settings/settings_lexer.l \
utils/utils.c utils/chunk.c utils/debug.c utils/enum.c utils/identification.c \
utils/lexparser.c utils/optionsfrom.c utils/capabilities.c utils/backtrace.c \
utils/parser_helper.c utils/test.c utils/process.c utils/utils/strerror.c

if !USE_WINDOWS
  libstrongswan_la_SOURCES += \
    threading/thread.c \
    threading/thread_value.c \
    threading/mutex.c \
    threading/rwlock.c \
    threading/spinlock.c \
    threading/semaphore.c \
    networking/streams/stream_unix.c \
    networking/streams/stream_service_unix.c
endif

# private header files
noinst_HEADERS = \
settings/settings_types.h

if USE_DEV_HEADERS
strongswan_includedir = ${dev_headers}
nobase_strongswan_include_HEADERS = \
library.h \
asn1/asn1.h asn1/asn1_parser.h asn1/oid.h bio/bio_reader.h bio/bio_writer.h \
collections/blocking_queue.h collections/enumerator.h collections/hashtable.h \
collections/linked_list.h collections/array.h collections/dictionary.h \
crypto/crypters/crypter.h crypto/hashers/hasher.h crypto/mac.h \
crypto/proposal/proposal_keywords.h crypto/proposal/proposal_keywords_static.h \
crypto/prfs/prf.h crypto/prfs/mac_prf.h crypto/rngs/rng.h crypto/nonce_gen.h \
crypto/prf_plus.h crypto/signers/signer.h crypto/signers/mac_signer.h \
crypto/crypto_factory.h crypto/crypto_tester.h crypto/diffie_hellman.h \
crypto/aead.h crypto/transform.h crypto/pkcs5.h crypto/iv/iv_gen.h \
crypto/iv/iv_gen_rand.h crypto/iv/iv_gen_seq.h \
credentials/credential_factory.h credentials/builder.h \
credentials/cred_encoding.h credentials/keys/private_key.h \
credentials/keys/public_key.h credentials/keys/shared_key.h \
credentials/certificates/certificate.h credentials/certificates/x509.h \
credentials/certificates/ac.h credentials/certificates/crl.h \
credentials/certificates/pkcs10.h credentials/certificates/ocsp_request.h \
credentials/certificates/ocsp_response.h \
credentials/certificates/pgp_certificate.h \
credentials/containers/container.h credentials/containers/pkcs7.h \
credentials/containers/pkcs12.h \
credentials/credential_manager.h credentials/sets/auth_cfg_wrapper.h \
credentials/sets/ocsp_response_wrapper.h credentials/sets/cert_cache.h \
credentials/sets/mem_cred.h credentials/sets/callback_cred.h \
credentials/auth_cfg.h credentials/credential_set.h credentials/cert_validator.h \
database/database.h database/database_factory.h fetcher/fetcher.h \
fetcher/fetcher_manager.h eap/eap.h pen/pen.h ipsec/ipsec_types.h \
networking/host.h networking/host_resolver.h networking/packet.h \
networking/tun_device.h networking/streams/stream.h \
networking/streams/stream_unix.h networking/streams/stream_service_unix.h \
networking/streams/stream_tcp.h networking/streams/stream_service_tcp.h \
networking/streams/stream_service.h networking/streams/stream_manager.h \
resolver/resolver.h resolver/resolver_response.h resolver/rr_set.h \
resolver/rr.h resolver/resolver_manager.h \
plugins/plugin_loader.h plugins/plugin.h plugins/plugin_feature.h \
processing/jobs/job.h processing/jobs/callback_job.h processing/processor.h \
processing/scheduler.h processing/watcher.h selectors/traffic_selector.h \
settings/settings.h threading/thread_value.h \
threading/thread.h threading/windows/thread.h \
threading/mutex.h threading/condvar.h threading/spinlock.h threading/semaphore.h \
threading/rwlock.h threading/rwlock_condvar.h threading/lock_profiler.h \
utils/utils.h utils/chunk.h utils/debug.h utils/enum.h utils/identification.h \
utils/lexparser.h utils/optionsfrom.h utils/capabilities.h utils/backtrace.h \
utils/leak_detective.h utils/printf_hook/printf_hook.h \
utils/printf_hook/printf_hook_vstr.h utils/printf_hook/printf_hook_builtin.h \
utils/parser_helper.h utils/test.h utils/integrity_checker.h utils/windows.h \
utils/process.h utils/utils/strerror.h
endif

library.lo :	$(top_builddir)/config.status

libstrongswan_la_LIBADD = $(DLLIB) $(BTLIB) $(SOCKLIB) $(RTLIB) $(BFDLIB) $(UNWINDLIB)

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-DIPSEC_DIR=\"${ipsecdir}\" \
	-DIPSEC_LIB_DIR=\"${ipseclibdir}\" \
	-DPLUGINDIR=\"${plugindir}\" \
	-DSTRONGSWAN_CONF=\"${strongswan_conf}\"

AM_CFLAGS = \
	@COVERAGE_CFLAGS@

AM_LDFLAGS = \
	-no-undefined

AM_YFLAGS = -v -d

if USE_WINDOWS
  libstrongswan_la_LIBADD += -lws2_32 -lpsapi
  libstrongswan_la_SOURCES += \
    threading/windows/thread.c \
    threading/windows/thread_value.c \
    threading/windows/mutex.c \
    threading/windows/rwlock.c \
    threading/windows/spinlock.c \
    threading/windows/semaphore.c \
    utils/windows.c
else
  libstrongswan_la_LIBADD += $(PTHREADLIB)
endif

if USE_DBGHELP
  libstrongswan_la_LIBADD += -ldbghelp
  AM_CPPFLAGS += -DHAVE_DBGHELP
endif

if USE_LEAK_DETECTIVE
  AM_CPPFLAGS += -DLEAK_DETECTIVE
  libstrongswan_la_SOURCES += utils/leak_detective.c
endif

if USE_LOCK_PROFILER
  AM_CPPFLAGS += -DLOCK_PROFILER
endif

if USE_INTEGRITY_TEST
  AM_CPPFLAGS += -DINTEGRITY_TEST
  libstrongswan_la_SOURCES += utils/integrity_checker.c
endif

if USE_VSTR
  libstrongswan_la_SOURCES += utils/printf_hook/printf_hook_vstr.c
  libstrongswan_la_LIBADD += -lvstr
endif

if USE_BUILTIN_PRINTF
  libstrongswan_la_SOURCES += utils/printf_hook/printf_hook_builtin.c
  libstrongswan_la_LIBADD += -lm
endif

if !USE_BUILTIN_PRINTF
if !USE_VSTR
  libstrongswan_la_SOURCES += utils/printf_hook/printf_hook_glibc.c
endif
endif

if USE_LIBCAP
  libstrongswan_la_LIBADD += -lcap
endif

EXTRA_DIST = \
asn1/oid.txt asn1/oid.pl \
crypto/proposal/proposal_keywords_static.txt \
Android.mk AndroidConfigLocal.h

BUILT_SOURCES = \
$(srcdir)/asn1/oid.c $(srcdir)/asn1/oid.h \
$(srcdir)/crypto/proposal/proposal_keywords_static.c \
settings/settings_parser.h

MAINTAINERCLEANFILES = \
$(srcdir)/asn1/oid.c $(srcdir)/asn1/oid.h \
$(srcdir)/crypto/proposal/proposal_keywords_static.c

$(srcdir)/asn1/oid.c :	$(srcdir)/asn1/oid.pl $(srcdir)/asn1/oid.txt
		$(AM_V_GEN) \
		(cd $(srcdir)/asn1/ && $(PERL) oid.pl)

$(srcdir)/asn1/oid.h :	$(srcdir)/asn1/oid.pl $(srcdir)/asn1/oid.txt
		$(AM_V_GEN) \
		(cd $(srcdir)/asn1/ && $(PERL) oid.pl)

$(srcdir)/crypto/proposal/proposal_keywords_static.c:	$(srcdir)/crypto/proposal/proposal_keywords_static.txt \
														$(srcdir)/crypto/proposal/proposal_keywords_static.h
		$(AM_V_GEN) \
		$(GPERF) -N proposal_get_token_static -m 10 -C -G -c -t -D < \
												$(srcdir)/crypto/proposal/proposal_keywords_static.txt > $@


# build plugins with their own Makefile
#######################################

if MONOLITHIC
SUBDIRS =
else
SUBDIRS = .
endif

if USE_AF_ALG
  SUBDIRS += plugins/af_alg
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/af_alg/libstrongswan-af-alg.la
endif
endif

if USE_AES
  SUBDIRS += plugins/aes
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/aes/libstrongswan-aes.la
endif
endif

if USE_DES
  SUBDIRS += plugins/des
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/des/libstrongswan-des.la
endif
endif

if USE_BLOWFISH
  SUBDIRS += plugins/blowfish
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/blowfish/libstrongswan-blowfish.la
endif
endif

if USE_RC2
  SUBDIRS += plugins/rc2
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/rc2/libstrongswan-rc2.la
endif
endif

if USE_MD4
  SUBDIRS += plugins/md4
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/md4/libstrongswan-md4.la
endif
endif

if USE_MD5
  SUBDIRS += plugins/md5
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/md5/libstrongswan-md5.la
endif
endif

if USE_SHA1
  SUBDIRS += plugins/sha1
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/sha1/libstrongswan-sha1.la
endif
endif

if USE_SHA2
  SUBDIRS += plugins/sha2
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/sha2/libstrongswan-sha2.la
endif
endif

if USE_GMP
  SUBDIRS += plugins/gmp
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/gmp/libstrongswan-gmp.la
endif
endif

if USE_RDRAND
  SUBDIRS += plugins/rdrand
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/rdrand/libstrongswan-rdrand.la
endif
endif

if USE_RANDOM
  SUBDIRS += plugins/random
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/random/libstrongswan-random.la
endif
endif

if USE_NONCE
  SUBDIRS += plugins/nonce
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/nonce/libstrongswan-nonce.la
endif
endif

if USE_HMAC
  SUBDIRS += plugins/hmac
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/hmac/libstrongswan-hmac.la
endif
endif

if USE_CMAC
  SUBDIRS += plugins/cmac
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/cmac/libstrongswan-cmac.la
endif
endif

if USE_XCBC
  SUBDIRS += plugins/xcbc
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/xcbc/libstrongswan-xcbc.la
endif
endif

if USE_X509
  SUBDIRS += plugins/x509
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/x509/libstrongswan-x509.la
endif
endif

if USE_REVOCATION
  SUBDIRS += plugins/revocation
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/revocation/libstrongswan-revocation.la
endif
endif

if USE_CONSTRAINTS
  SUBDIRS += plugins/constraints
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/constraints/libstrongswan-constraints.la
endif
endif

if USE_ACERT
  SUBDIRS += plugins/acert
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/acert/libstrongswan-acert.la
endif
endif

if USE_PUBKEY
  SUBDIRS += plugins/pubkey
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/pubkey/libstrongswan-pubkey.la
endif
endif

if USE_PKCS1
  SUBDIRS += plugins/pkcs1
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/pkcs1/libstrongswan-pkcs1.la
endif
endif

if USE_PKCS7
  SUBDIRS += plugins/pkcs7
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/pkcs7/libstrongswan-pkcs7.la
endif
endif

if USE_PKCS8
  SUBDIRS += plugins/pkcs8
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/pkcs8/libstrongswan-pkcs8.la
endif
endif

if USE_PKCS12
  SUBDIRS += plugins/pkcs12
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/pkcs12/libstrongswan-pkcs12.la
endif
endif

if USE_PGP
  SUBDIRS += plugins/pgp
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/pgp/libstrongswan-pgp.la
endif
endif

if USE_DNSKEY
  SUBDIRS += plugins/dnskey
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/dnskey/libstrongswan-dnskey.la
endif
endif

if USE_SSHKEY
  SUBDIRS += plugins/sshkey
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/sshkey/libstrongswan-sshkey.la
endif
endif

if USE_PEM
  SUBDIRS += plugins/pem
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/pem/libstrongswan-pem.la
endif
endif

if USE_CURL
  SUBDIRS += plugins/curl
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/curl/libstrongswan-curl.la
endif
endif

if USE_WINHTTP
  SUBDIRS += plugins/winhttp
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/winhttp/libstrongswan-winhttp.la
endif
endif

if USE_UNBOUND
  SUBDIRS += plugins/unbound
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/unbound/libstrongswan-unbound.la
endif
endif

if USE_SOUP
  SUBDIRS += plugins/soup
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/soup/libstrongswan-soup.la
endif
endif

if USE_LDAP
  SUBDIRS += plugins/ldap
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/ldap/libstrongswan-ldap.la
endif
endif

if USE_MYSQL
  SUBDIRS += plugins/mysql
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/mysql/libstrongswan-mysql.la
endif
endif

if USE_SQLITE
  SUBDIRS += plugins/sqlite
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/sqlite/libstrongswan-sqlite.la
endif
endif

if USE_PADLOCK
  SUBDIRS += plugins/padlock
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/padlock/libstrongswan-padlock.la
endif
endif

if USE_OPENSSL
  SUBDIRS += plugins/openssl
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/openssl/libstrongswan-openssl.la
endif
endif

if USE_GCRYPT
  SUBDIRS += plugins/gcrypt
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/gcrypt/libstrongswan-gcrypt.la
endif
endif

if USE_FIPS_PRF
  SUBDIRS += plugins/fips_prf
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/fips_prf/libstrongswan-fips-prf.la
endif
endif

if USE_AGENT
  SUBDIRS += plugins/agent
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/agent/libstrongswan-agent.la
endif
endif

if USE_KEYCHAIN
  SUBDIRS += plugins/keychain
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/keychain/libstrongswan-keychain.la
endif
endif

if USE_PKCS11
  SUBDIRS += plugins/pkcs11
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/pkcs11/libstrongswan-pkcs11.la
endif
endif

if USE_CTR
  SUBDIRS += plugins/ctr
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/ctr/libstrongswan-ctr.la
endif
endif

if USE_CCM
  SUBDIRS += plugins/ccm
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/ccm/libstrongswan-ccm.la
endif
endif

if USE_GCM
  SUBDIRS += plugins/gcm
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/gcm/libstrongswan-gcm.la
endif
endif

if USE_NTRU
  SUBDIRS += plugins/ntru
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/ntru/libstrongswan-ntru.la
endif
endif

if USE_TEST_VECTORS
  SUBDIRS += plugins/test_vectors
if MONOLITHIC
  libstrongswan_la_LIBADD += plugins/test_vectors/libstrongswan-test-vectors.la
endif
endif

if MONOLITHIC
  SUBDIRS += .
endif
SUBDIRS += tests
