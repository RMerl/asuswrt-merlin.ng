/*
 * Copyright (C) 2009 Martin Willi
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

#include "gcrypt_plugin.h"

#include "gcrypt_hasher.h"
#include "gcrypt_crypter.h"
#include "gcrypt_rng.h"
#include "gcrypt_dh.h"
#include "gcrypt_rsa_private_key.h"
#include "gcrypt_rsa_public_key.h"

#include <library.h>
#include <utils/debug.h>
#include <threading/mutex.h>

#include <errno.h>
#include <gcrypt.h>
#include <pthread.h>

typedef struct private_gcrypt_plugin_t private_gcrypt_plugin_t;

/**
 * private data of gcrypt_plugin
 */
struct private_gcrypt_plugin_t {

	/**
	 * public functions
	 */
	gcrypt_plugin_t public;
};

/**
 * Define gcrypt multi-threading callbacks as gcry_threads_pthread
 */
GCRY_THREAD_OPTION_PTHREAD_IMPL;

METHOD(plugin_t, get_name, char*,
	private_gcrypt_plugin_t *this)
{
	return "gcrypt";
}

METHOD(plugin_t, get_features, int,
	private_gcrypt_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		/* we provide threading-safe initialization of libgcrypt */
		PLUGIN_PROVIDE(CUSTOM, "gcrypt-threading"),
		/* crypters */
		PLUGIN_REGISTER(CRYPTER, gcrypt_crypter_create),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CTR, 32),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_AES_CBC, 32),
			/* gcrypt only supports 128 bit blowfish */
			PLUGIN_PROVIDE(CRYPTER, ENCR_BLOWFISH, 16),
#ifdef HAVE_GCRY_CIPHER_CAMELLIA
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CTR, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CTR, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CTR, 32),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAMELLIA_CBC, 32),
#endif
			PLUGIN_PROVIDE(CRYPTER, ENCR_CAST, 0),
			PLUGIN_PROVIDE(CRYPTER, ENCR_3DES, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_DES, 8),
			PLUGIN_PROVIDE(CRYPTER, ENCR_DES_ECB, 8),
			PLUGIN_PROVIDE(CRYPTER, ENCR_SERPENT_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_SERPENT_CBC, 24),
			PLUGIN_PROVIDE(CRYPTER, ENCR_SERPENT_CBC, 32),
			PLUGIN_PROVIDE(CRYPTER, ENCR_TWOFISH_CBC, 16),
			PLUGIN_PROVIDE(CRYPTER, ENCR_TWOFISH_CBC, 32),
		/* hashers */
		PLUGIN_REGISTER(HASHER, gcrypt_hasher_create),
			PLUGIN_PROVIDE(HASHER, HASH_MD4),
			PLUGIN_PROVIDE(HASHER, HASH_MD5),
			PLUGIN_PROVIDE(HASHER, HASH_SHA1),
			PLUGIN_PROVIDE(HASHER, HASH_SHA224),
			PLUGIN_PROVIDE(HASHER, HASH_SHA256),
			PLUGIN_PROVIDE(HASHER, HASH_SHA384),
			PLUGIN_PROVIDE(HASHER, HASH_SHA512),
		/* MODP DH groups */
		PLUGIN_REGISTER(DH, gcrypt_dh_create),
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
		PLUGIN_REGISTER(DH, gcrypt_dh_create_custom),
			PLUGIN_PROVIDE(DH, MODP_CUSTOM),
		/* RSA private/public key loading */
		PLUGIN_REGISTER(PUBKEY, gcrypt_rsa_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
		PLUGIN_REGISTER(PRIVKEY, gcrypt_rsa_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),
		PLUGIN_REGISTER(PRIVKEY_GEN, gcrypt_rsa_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_RSA),
		/* random numbers */
		PLUGIN_REGISTER(RNG, gcrypt_rng_create),
			PLUGIN_PROVIDE(RNG, RNG_WEAK),
			PLUGIN_PROVIDE(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(RNG, RNG_TRUE),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_gcrypt_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *gcrypt_plugin_create()
{
	private_gcrypt_plugin_t *this;

	gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);

	if (!gcry_check_version(GCRYPT_VERSION))
	{
		DBG1(DBG_LIB, "libgcrypt version mismatch");
		return NULL;
	}

	/* we currently do not use secure memory */
	gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
	if (lib->settings->get_bool(lib->settings, "%s.plugins.gcrypt.quick_random",
								FALSE, lib->ns))
	{
		gcry_control(GCRYCTL_ENABLE_QUICK_RANDOM, 0);
	}
	gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
