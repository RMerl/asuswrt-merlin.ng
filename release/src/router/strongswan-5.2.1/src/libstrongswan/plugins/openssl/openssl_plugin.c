/*
 * Copyright (C) 2008-2013 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <library.h>
#include <utils/debug.h>
#include <threading/thread.h>
#include <threading/mutex.h>
#include <threading/thread_value.h>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include "openssl_plugin.h"
#include "openssl_util.h"
#include "openssl_crypter.h"
#include "openssl_hasher.h"
#include "openssl_sha1_prf.h"
#include "openssl_diffie_hellman.h"
#include "openssl_ec_diffie_hellman.h"
#include "openssl_rsa_private_key.h"
#include "openssl_rsa_public_key.h"
#include "openssl_ec_private_key.h"
#include "openssl_ec_public_key.h"
#include "openssl_x509.h"
#include "openssl_crl.h"
#include "openssl_pkcs7.h"
#include "openssl_pkcs12.h"
#include "openssl_rng.h"
#include "openssl_hmac.h"
#include "openssl_gcm.h"

#ifndef FIPS_MODE
#define FIPS_MODE 0
#endif

typedef struct private_openssl_plugin_t private_openssl_plugin_t;

/**
 * private data of openssl_plugin
 */
struct private_openssl_plugin_t {

	/**
	 * public functions
	 */
	openssl_plugin_t public;
};

/**
 * Array of static mutexs, with CRYPTO_num_locks() mutex
 */
static mutex_t **mutex = NULL;

/**
 * Locking callback for static locks
 */
static void locking_function(int mode, int type, const char *file, int line)
{
	if (mutex)
	{
		if (mode & CRYPTO_LOCK)
		{
			mutex[type]->lock(mutex[type]);
		}
		else
		{
			mutex[type]->unlock(mutex[type]);
		}
	}
}

/**
 * Implementation of dynlock
 */
struct CRYPTO_dynlock_value {
	mutex_t *mutex;
};

/**
 * Callback to create a dynamic lock
 */
static struct CRYPTO_dynlock_value *create_function(const char *file, int line)
{
	struct CRYPTO_dynlock_value *lock;

	lock = malloc_thing(struct CRYPTO_dynlock_value);
	lock->mutex = mutex_create(MUTEX_TYPE_DEFAULT);
	return lock;
}

/**
 * Callback to (un-)lock a dynamic lock
 */
static void lock_function(int mode, struct CRYPTO_dynlock_value *lock,
						  const char *file, int line)
{
	if (mode & CRYPTO_LOCK)
	{
		lock->mutex->lock(lock->mutex);
	}
	else
	{
		lock->mutex->unlock(lock->mutex);
	}
}

/**
 * Callback to destroy a dynamic lock
 */
static void destroy_function(struct CRYPTO_dynlock_value *lock,
							 const char *file, int line)
{
	lock->mutex->destroy(lock->mutex);
	free(lock);
}

/**
 * Thread-local value used to cleanup thread-specific error buffers
 */
static thread_value_t *cleanup;

/**
 * Called when a thread is destroyed. Avoid recursion by setting the thread id
 * explicitly.
 */
static void cleanup_thread(void *arg)
{
#if OPENSSL_VERSION_NUMBER >= 0x1000000fL
	CRYPTO_THREADID tid;

	CRYPTO_THREADID_set_numeric(&tid, (u_long)(uintptr_t)arg);
	ERR_remove_thread_state(&tid);
#else
	ERR_remove_state((u_long)(uintptr_t)arg);
#endif
}

/**
 * Thread-ID callback function
 */
static u_long id_function(void)
{
	u_long id;

	/* ensure the thread ID is never zero, otherwise OpenSSL might try to
	 * acquire locks recursively */
	id = 1 + (u_long)thread_current_id();

	/* cleanup a thread's state later if OpenSSL interacted with it */
	cleanup->set(cleanup, (void*)(uintptr_t)id);
	return id;
}

#if OPENSSL_VERSION_NUMBER >= 0x1000000fL
/**
 * Callback for thread ID
 */
static void threadid_function(CRYPTO_THREADID *threadid)
{
	CRYPTO_THREADID_set_numeric(threadid, id_function());
}
#endif /* OPENSSL_VERSION_NUMBER */

/**
 * initialize OpenSSL for multi-threaded use
 */
static void threading_init()
{
	int i, num_locks;

	cleanup = thread_value_create(cleanup_thread);

#if OPENSSL_VERSION_NUMBER >= 0x1000000fL
	CRYPTO_THREADID_set_callback(threadid_function);
#else
	CRYPTO_set_id_callback(id_function);
#endif

	CRYPTO_set_locking_callback(locking_function);

	CRYPTO_set_dynlock_create_callback(create_function);
	CRYPTO_set_dynlock_lock_callback(lock_function);
	CRYPTO_set_dynlock_destroy_callback(destroy_function);

	num_locks = CRYPTO_num_locks();
	mutex = malloc(sizeof(mutex_t*) * num_locks);
	for (i = 0; i < num_locks; i++)
	{
		mutex[i] = mutex_create(MUTEX_TYPE_DEFAULT);
	}
}

/**
 * cleanup OpenSSL threading locks
 */
static void threading_cleanup()
{
	int i, num_locks;

	num_locks = CRYPTO_num_locks();
	for (i = 0; i < num_locks; i++)
	{
		mutex[i]->destroy(mutex[i]);
	}
	free(mutex);
	mutex = NULL;

	cleanup->destroy(cleanup);
}

/**
 * Seed the OpenSSL RNG, if required
 */
static bool seed_rng()
{
	rng_t *rng = NULL;
	char buf[32];

	while (RAND_status() != 1)
	{
		if (!rng)
		{
			rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
			if (!rng)
			{
				return FALSE;
			}
		}
		if (!rng->get_bytes(rng, sizeof(buf), buf))
		{
			rng->destroy(rng);
			return FALSE;
		}
		RAND_seed(buf, sizeof(buf));
	}
	DESTROY_IF(rng);
	return TRUE;
}

METHOD(plugin_t, get_name, char*,
	private_openssl_plugin_t *this)
{
	return "openssl";
}

METHOD(plugin_t, get_features, int,
	private_openssl_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		/* we provide OpenSSL threading callbacks */
		PLUGIN_PROVIDE(CUSTOM, "openssl-threading"),
		/* crypters */
		PLUGIN_REGISTER(CRYPTER, openssl_crypter_create),
#ifndef OPENSSL_NO_AES
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 32),
#endif
#ifndef OPENSSL_NO_CAMELLIA
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CBC, 32),
#endif
#ifndef OPENSSL_NO_RC5
			PLUGIN_PROVIDE(CRYPTER, ENCR_RC5, 0),
#endif
#ifndef OPENSSL_NO_CAST
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAST, 0),
#endif
#ifndef OPENSSL_NO_BLOWFISH
			PLUGIN_PROVIDE(CRYPTER, ENCR_BLOWFISH, 0),
#endif
#ifndef OPENSSL_NO_IDEA
			PLUGIN_PROVIDE(CRYPTER, ENCR_IDEA, 16),
#endif
#ifndef OPENSSL_NO_DES
			PLUGIN_PROVIDE(CRYPTER, ENCR_3DES, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_DES, 8),
			PLUGIN_PROVIDE(CRYPTER, ENCR_DES_ECB, 8),
#endif
			PLUGIN_PROVIDE(CRYPTER, ENCR_NULL, 0),
		/* hashers */
		PLUGIN_REGISTER(HASHER, openssl_hasher_create),
#ifndef OPENSSL_NO_MD2
			PLUGIN_PROVIDE(HASHER, HASH_MD2),
#endif
#ifndef OPENSSL_NO_MD4
			PLUGIN_PROVIDE(HASHER, HASH_MD4),
#endif
#ifndef OPENSSL_NO_MD5
			PLUGIN_PROVIDE(HASHER, HASH_MD5),
#endif
#ifndef OPENSSL_NO_SHA1
			PLUGIN_PROVIDE(HASHER, HASH_SHA1),
#endif
#ifndef OPENSSL_NO_SHA256
			PLUGIN_PROVIDE(HASHER, HASH_SHA224),
			PLUGIN_PROVIDE(HASHER, HASH_SHA256),
#endif
#ifndef OPENSSL_NO_SHA512
			PLUGIN_PROVIDE(HASHER, HASH_SHA384),
			PLUGIN_PROVIDE(HASHER, HASH_SHA512),
#endif
#ifndef OPENSSL_NO_SHA1
		/* keyed sha1 hasher (aka prf) */
		PLUGIN_REGISTER(PRF, openssl_sha1_prf_create),
			PLUGIN_PROVIDE(PRF, PRF_KEYED_SHA1),
#endif
#ifndef OPENSSL_NO_HMAC
		PLUGIN_REGISTER(PRF, openssl_hmac_prf_create),
#ifndef OPENSSL_NO_MD5
			PLUGIN_PROVIDE(PRF, PRF_HMAC_MD5),
#endif
#ifndef OPENSSL_NO_SHA1
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA1),
#endif
#ifndef OPENSSL_NO_SHA256
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_256),
#endif
#ifndef OPENSSL_NO_SHA512
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_384),
			PLUGIN_PROVIDE(PRF, PRF_HMAC_SHA2_512),
#endif
		PLUGIN_REGISTER(SIGNER, openssl_hmac_signer_create),
#ifndef OPENSSL_NO_MD5
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_MD5_96),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_MD5_128),
#endif
#ifndef OPENSSL_NO_SHA1
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_96),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_128),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA1_160),
#endif
#ifndef OPENSSL_NO_SHA256
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_256_128),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_256_256),
#endif
#ifndef OPENSSL_NO_SHA512
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_384_192),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_384_384),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_512_256),
			PLUGIN_PROVIDE(SIGNER, AUTH_HMAC_SHA2_512_512),
#endif
#endif /* OPENSSL_NO_HMAC */
#if OPENSSL_VERSION_NUMBER >= 0x1000100fL
#ifndef OPENSSL_NO_AES
		/* AES GCM */
		PLUGIN_REGISTER(AEAD, openssl_gcm_create),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV8, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV12, 32),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 16),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 24),
			PLUGIN_PROVIDE(AEAD, ENCR_AES_GCM_ICV16, 32),
#endif /* OPENSSL_NO_AES */
#endif /* OPENSSL_VERSION_NUMBER */
#ifndef OPENSSL_NO_DH
		/* MODP DH groups */
		PLUGIN_REGISTER(DH, openssl_diffie_hellman_create),
			PLUGIN_PROVIDE(DH, MODP_2048_BIT),
			PLUGIN_PROVIDE(DH, MODP_2048_224),
			PLUGIN_PROVIDE(DH, MODP_2048_256),
			PLUGIN_PROVIDE(DH, MODP_1536_BIT),
			PLUGIN_PROVIDE(DH, MODP_3072_BIT),
			PLUGIN_PROVIDE(DH, MODP_4096_BIT),
			PLUGIN_PROVIDE(DH, MODP_6144_BIT),
			PLUGIN_PROVIDE(DH, MODP_8192_BIT),
			PLUGIN_PROVIDE(DH, MODP_1024_BIT),
			PLUGIN_PROVIDE(DH, MODP_1024_160),
			PLUGIN_PROVIDE(DH, MODP_768_BIT),
			PLUGIN_PROVIDE(DH, MODP_CUSTOM),
#endif
#ifndef OPENSSL_NO_RSA
		/* RSA private/public key loading */
		PLUGIN_REGISTER(PRIVKEY, openssl_rsa_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),
		PLUGIN_REGISTER(PRIVKEY, openssl_rsa_private_key_connect, FALSE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
		PLUGIN_REGISTER(PRIVKEY_GEN, openssl_rsa_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_RSA),
		PLUGIN_REGISTER(PUBKEY, openssl_rsa_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
		PLUGIN_REGISTER(PUBKEY, openssl_rsa_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_ANY),
		/* signature/encryption schemes */
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_NULL),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_NULL),
#ifndef OPENSSL_NO_SHA1
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA1),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA1),
#endif
#ifndef OPENSSL_NO_SHA256
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA224),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA256),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA224),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA256),
#endif
#ifndef OPENSSL_NO_SHA512
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA384),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA384),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA512),
#endif
#ifndef OPENSSL_NO_MD5
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_MD5),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_MD5),
#endif
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_PKCS1),
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT, ENCRYPT_RSA_PKCS1),
#endif /* OPENSSL_NO_RSA */
		/* certificate/CRL loading */
		PLUGIN_REGISTER(CERT_DECODE, openssl_x509_load, TRUE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_X509),
				PLUGIN_SDEPEND(PUBKEY, KEY_RSA),
				PLUGIN_SDEPEND(PUBKEY, KEY_ECDSA),
				PLUGIN_SDEPEND(PUBKEY, KEY_DSA),
		PLUGIN_REGISTER(CERT_DECODE, openssl_crl_load, TRUE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_X509_CRL),
#if OPENSSL_VERSION_NUMBER >= 0x0090807fL
#ifndef OPENSSL_NO_CMS
		PLUGIN_REGISTER(CONTAINER_DECODE, openssl_pkcs7_load, TRUE),
			PLUGIN_PROVIDE(CONTAINER_DECODE, CONTAINER_PKCS7),
#endif /* OPENSSL_NO_CMS */
#endif /* OPENSSL_VERSION_NUMBER */
		PLUGIN_REGISTER(CONTAINER_DECODE, openssl_pkcs12_load, TRUE),
			PLUGIN_PROVIDE(CONTAINER_DECODE, CONTAINER_PKCS12),
#ifndef OPENSSL_NO_ECDH
		/* EC DH groups */
		PLUGIN_REGISTER(DH, openssl_ec_diffie_hellman_create),
			PLUGIN_PROVIDE(DH, ECP_256_BIT),
			PLUGIN_PROVIDE(DH, ECP_384_BIT),
			PLUGIN_PROVIDE(DH, ECP_521_BIT),
			PLUGIN_PROVIDE(DH, ECP_224_BIT),
			PLUGIN_PROVIDE(DH, ECP_192_BIT),
			PLUGIN_PROVIDE(DH, ECP_224_BP),
			PLUGIN_PROVIDE(DH, ECP_256_BP),
			PLUGIN_PROVIDE(DH, ECP_384_BP),
			PLUGIN_PROVIDE(DH, ECP_512_BP),
#endif
#ifndef OPENSSL_NO_ECDSA
		/* EC private/public key loading */
		PLUGIN_REGISTER(PRIVKEY, openssl_ec_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ECDSA),
		PLUGIN_REGISTER(PRIVKEY_GEN, openssl_ec_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_ECDSA),
		PLUGIN_REGISTER(PUBKEY, openssl_ec_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_ECDSA),
		/* signature encryption schemes */
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_NULL),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_NULL),
#ifndef OPENSSL_NO_SHA1
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA1_DER),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA1_DER),
#endif
#ifndef OPENSSL_NO_SHA256
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA256_DER),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA256_DER),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_256),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_256),
#endif
#ifndef OPENSSL_NO_SHA512
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA384_DER),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_WITH_SHA512_DER),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA384_DER),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_WITH_SHA512_DER),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_384),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_ECDSA_521),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_384),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_ECDSA_521),
#endif
#endif /* OPENSSL_NO_ECDSA */
		PLUGIN_REGISTER(RNG, openssl_rng_create),
			PLUGIN_PROVIDE(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(RNG, RNG_WEAK),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_openssl_plugin_t *this)
{
	CONF_modules_free();
	OBJ_cleanup();
	EVP_cleanup();
#ifndef OPENSSL_NO_ENGINE
	ENGINE_cleanup();
#endif /* OPENSSL_NO_ENGINE */
	CRYPTO_cleanup_all_ex_data();
	threading_cleanup();
	ERR_free_strings();

	free(this);
}

/*
 * see header file
 */
plugin_t *openssl_plugin_create()
{
	private_openssl_plugin_t *this;
	int fips_mode;

	fips_mode = lib->settings->get_int(lib->settings,
							"%s.plugins.openssl.fips_mode", FIPS_MODE, lib->ns);
#ifdef OPENSSL_FIPS
	if (fips_mode)
	{
		if (FIPS_mode() != fips_mode && !FIPS_mode_set(fips_mode))
		{
			DBG1(DBG_LIB, "unable to set openssl FIPS mode(%d) from (%d)",
				 fips_mode, FIPS_mode());
			return NULL;
		}
	}
#else
	if (fips_mode)
	{
		DBG1(DBG_LIB, "openssl FIPS mode(%d) unavailable", fips_mode);
		return NULL;
	}
#endif

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	threading_init();

	OPENSSL_config(NULL);
	OpenSSL_add_all_algorithms();

#ifdef OPENSSL_FIPS
	/* we do this here as it may have been enabled via openssl.conf */
	fips_mode = FIPS_mode();
	dbg(DBG_LIB, strpfx(lib->ns, "charon") ? 1 : 2,
		"openssl FIPS mode(%d) - %sabled ", fips_mode, fips_mode ? "en" : "dis");
#endif /* OPENSSL_FIPS */

#ifndef OPENSSL_NO_ENGINE
	/* activate support for hardware accelerators */
	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();
#endif /* OPENSSL_NO_ENGINE */

	if (!seed_rng())
	{
		DBG1(DBG_CFG, "no RNG found to seed OpenSSL");
		destroy(this);
		return NULL;
	}

	return &this->public.plugin;
}
