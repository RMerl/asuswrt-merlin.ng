LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
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
settings/settings_parser.c settings/settings_lexer.c \
utils/utils.c utils/chunk.c utils/debug.c utils/enum.c utils/identification.c \
utils/lexparser.c utils/optionsfrom.c utils/capabilities.c utils/backtrace.c \
utils/parser_helper.c utils/test.c utils/process.c utils/utils/strerror.c

libstrongswan_la_SOURCES += \
    threading/thread.c \
    threading/thread_value.c \
    threading/mutex.c \
    threading/rwlock.c \
    threading/spinlock.c \
    threading/semaphore.c \
    networking/streams/stream_unix.c \
    networking/streams/stream_service_unix.c

libstrongswan_la_SOURCES += utils/printf_hook/printf_hook_builtin.c

LOCAL_SRC_FILES := $(libstrongswan_la_SOURCES)

# adding the plugin source files

LOCAL_SRC_FILES += $(call add_plugin, aes)

LOCAL_SRC_FILES += $(call add_plugin, curl)
ifneq ($(call plugin_enabled, curl),)
LOCAL_C_INCLUDES += $(libcurl_PATH)
LOCAL_SHARED_LIBRARIES += libcurl
endif

LOCAL_SRC_FILES += $(call add_plugin, des)

LOCAL_SRC_FILES += $(call add_plugin, fips-prf)

LOCAL_SRC_FILES += $(call add_plugin, gmp)
ifneq ($(call plugin_enabled, gmp),)
LOCAL_C_INCLUDES += $(libgmp_PATH)
LOCAL_SHARED_LIBRARIES += libgmp
endif

LOCAL_SRC_FILES += $(call add_plugin, hmac)

LOCAL_SRC_FILES += $(call add_plugin, md4)

LOCAL_SRC_FILES += $(call add_plugin, md5)

LOCAL_SRC_FILES += $(call add_plugin, nonce)

LOCAL_SRC_FILES += $(call add_plugin, openssl)
ifneq ($(call plugin_enabled, openssl),)
LOCAL_C_INCLUDES += $(openssl_PATH)
LOCAL_STATIC_LIBRARIES += libcrypto_static
endif

LOCAL_SRC_FILES += $(call add_plugin, pem)

LOCAL_SRC_FILES += $(call add_plugin, pkcs1)

LOCAL_SRC_FILES += $(call add_plugin, pkcs7)

LOCAL_SRC_FILES += $(call add_plugin, pkcs8)

LOCAL_SRC_FILES += $(call add_plugin, pkcs11)

LOCAL_SRC_FILES += $(call add_plugin, pubkey)

LOCAL_SRC_FILES += $(call add_plugin, random)

LOCAL_SRC_FILES += $(call add_plugin, sha1)

LOCAL_SRC_FILES += $(call add_plugin, sha2)

LOCAL_SRC_FILES += $(call add_plugin, x509)

LOCAL_SRC_FILES += $(call add_plugin, xcbc)

# build libstrongswan ----------------------------------------------------------

LOCAL_CFLAGS := $(strongswan_CFLAGS) \
	-include $(LOCAL_PATH)/AndroidConfigLocal.h

LOCAL_MODULE := libstrongswan

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES += libdl

include $(BUILD_SHARED_LIBRARY)
