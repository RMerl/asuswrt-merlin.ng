/*
 * Copyright (C) 2008-2009 Martin Willi
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

#include "gmp_plugin.h"

#include <library.h>
#include "gmp_diffie_hellman.h"
#include "gmp_rsa_private_key.h"
#include "gmp_rsa_public_key.h"

typedef struct private_gmp_plugin_t private_gmp_plugin_t;

/**
 * private data of gmp_plugin
 */
struct private_gmp_plugin_t {

	/**
	 * public functions
	 */
	gmp_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_gmp_plugin_t *this)
{
	return "gmp";
}

METHOD(plugin_t, get_features, int,
	private_gmp_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		/* DH groups */
		PLUGIN_REGISTER(DH, gmp_diffie_hellman_create),
			PLUGIN_PROVIDE(DH, MODP_2048_BIT),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_2048_224),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_2048_256),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_1536_BIT),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_3072_BIT),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_4096_BIT),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_6144_BIT),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_8192_BIT),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_1024_BIT),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_1024_160),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(DH, MODP_768_BIT),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
		PLUGIN_REGISTER(DH, gmp_diffie_hellman_create_custom),
			PLUGIN_PROVIDE(DH, MODP_CUSTOM),
				PLUGIN_DEPENDS(RNG, RNG_STRONG),
			/* private/public keys */
		PLUGIN_REGISTER(PRIVKEY, gmp_rsa_private_key_load, TRUE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),
		PLUGIN_REGISTER(PRIVKEY_GEN, gmp_rsa_private_key_gen, FALSE),
			PLUGIN_PROVIDE(PRIVKEY_GEN, KEY_RSA),
				PLUGIN_DEPENDS(RNG, RNG_TRUE),
		PLUGIN_REGISTER(PUBKEY, gmp_rsa_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
		/* signature schemes, private */
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_NULL),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA1),
			PLUGIN_DEPENDS(HASHER, HASH_SHA1),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA224),
			PLUGIN_DEPENDS(HASHER, HASH_SHA224),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA256),
			PLUGIN_DEPENDS(HASHER, HASH_SHA256),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA384),
			PLUGIN_DEPENDS(HASHER, HASH_SHA384),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_SHA512),
			PLUGIN_DEPENDS(HASHER, HASH_SHA512),
		PLUGIN_PROVIDE(PRIVKEY_SIGN, SIGN_RSA_EMSA_PKCS1_MD5),
			PLUGIN_DEPENDS(HASHER, HASH_MD5),
		/* signature verification schemes */
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_NULL),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA1),
			PLUGIN_DEPENDS(HASHER, HASH_SHA1),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA224),
			PLUGIN_DEPENDS(HASHER, HASH_SHA224),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA256),
			PLUGIN_DEPENDS(HASHER, HASH_SHA256),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA384),
			PLUGIN_DEPENDS(HASHER, HASH_SHA384),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA512),
			PLUGIN_DEPENDS(HASHER, HASH_SHA512),
		PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_MD5),
			PLUGIN_DEPENDS(HASHER, HASH_MD5),
		/* en-/decryption schemes */
		PLUGIN_PROVIDE(PRIVKEY_DECRYPT, ENCRYPT_RSA_PKCS1),
		PLUGIN_PROVIDE(PUBKEY_ENCRYPT, ENCRYPT_RSA_PKCS1),
			PLUGIN_DEPENDS(RNG, RNG_WEAK),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_gmp_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *gmp_plugin_create()
{
	private_gmp_plugin_t *this;

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

