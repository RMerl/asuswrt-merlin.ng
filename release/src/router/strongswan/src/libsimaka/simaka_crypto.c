/*
 * Copyright (C) 2009-2011 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#include "simaka_crypto.h"

#include "simaka_manager.h"

#include <utils/debug.h>

/** length of the k_encr key */
#define KENCR_LEN 16
/** length of the k_auth key */
#define KAUTH_LEN 16
/** length of the MSK */
#define MSK_LEN 64
/** length of the EMSK */
#define EMSK_LEN 64

typedef struct private_simaka_crypto_t private_simaka_crypto_t;

/**
 * Private data of an simaka_crypto_t object.
 */
struct private_simaka_crypto_t {

	/**
	 * Public simaka_crypto_t interface.
	 */
	simaka_crypto_t public;

	/**
	 * EAP type this crypto is used, SIM or AKA
	 */
	eap_type_t type;

	/**
	 * signer to create/verify AT_MAC
	 */
	signer_t *signer;

	/**
	 * crypter to encrypt/decrypt AT_ENCR_DATA
	 */
	crypter_t *crypter;

	/**
	 * hasher used in key derivation
	 */
	hasher_t *hasher;

	/**
	 * PRF function used in key derivation
	 */
	prf_t *prf;

	/**
	 * Random number generator to generate nonces
	 */
	rng_t *rng;

	/**
	 * Have k_encr/k_auth been derived?
	 */
	bool derived;
};

METHOD(simaka_crypto_t, get_signer, signer_t*,
	private_simaka_crypto_t *this)
{
	return this->derived ? this->signer : NULL;
}

METHOD(simaka_crypto_t, get_crypter, crypter_t*,
	private_simaka_crypto_t *this)
{
	return this->derived ? this->crypter : NULL;
}

METHOD(simaka_crypto_t, get_rng, rng_t*,
	private_simaka_crypto_t *this)
{
	return this->rng;
}

/**
 * Call SIM/AKA key hook
 */
static void call_hook(private_simaka_crypto_t *this, chunk_t encr, chunk_t auth)
{
	simaka_manager_t *mgr;

	switch (this->type)
	{
		case EAP_SIM:
			mgr = lib->get(lib, "sim-manager");
			break;
		case EAP_AKA:
			mgr = lib->get(lib, "aka-manager");
			break;
		default:
			return;
	}
	mgr->key_hook(mgr, encr, auth);
}

METHOD(simaka_crypto_t, derive_keys_full, bool,
	private_simaka_crypto_t *this, identification_t *id,
	chunk_t data, chunk_t *mk, chunk_t *msk)
{
	chunk_t str, k_encr, k_auth;
	int i;

	/* For SIM: MK = SHA1(Identity|n*Kc|NONCE_MT|Version List|Selected Version)
	 * For AKA: MK = SHA1(Identity|IK|CK) */
	if (!this->hasher->get_hash(this->hasher, id->get_encoding(id), NULL) ||
		!this->hasher->allocate_hash(this->hasher, data, mk))
	{
		return FALSE;
	}
	DBG3(DBG_LIB, "MK %B", mk);

	/* K_encr | K_auth | MSK | EMSK = prf() | prf() | prf() | prf() */
	if (!this->prf->set_key(this->prf, *mk))
	{
		chunk_clear(mk);
		return FALSE;
	}
	str = chunk_alloca(this->prf->get_block_size(this->prf) * 3);
	for (i = 0; i < 3; i++)
	{
		if (!this->prf->get_bytes(this->prf, chunk_empty,
								  str.ptr + str.len / 3 * i))
		{
			chunk_clear(mk);
			return FALSE;
		}
	}

	k_encr = chunk_create(str.ptr, KENCR_LEN);
	k_auth = chunk_create(str.ptr + KENCR_LEN, KAUTH_LEN);

	if (!this->signer->set_key(this->signer, k_auth) ||
		!this->crypter->set_key(this->crypter, k_encr))
	{
		chunk_clear(mk);
		return FALSE;
	}

	*msk = chunk_clone(chunk_create(str.ptr + KENCR_LEN + KAUTH_LEN, MSK_LEN));
	DBG3(DBG_LIB, "K_encr %B\nK_auth %B\nMSK %B", &k_encr, &k_auth, msk);

	call_hook(this, k_encr, k_auth);

	this->derived = TRUE;
	return TRUE;
}

METHOD(simaka_crypto_t, derive_keys_reauth, bool,
	private_simaka_crypto_t *this, chunk_t mk)
{
	chunk_t str, k_encr, k_auth;
	int i;

	/* K_encr | K_auth = prf() | prf() */
	if (!this->prf->set_key(this->prf, mk))
	{
		return FALSE;
	}
	str = chunk_alloca(this->prf->get_block_size(this->prf) * 2);
	for (i = 0; i < 2; i++)
	{
		if (!this->prf->get_bytes(this->prf, chunk_empty,
								  str.ptr + str.len / 2 * i))
		{
			return FALSE;
		}
	}
	k_encr = chunk_create(str.ptr, KENCR_LEN);
	k_auth = chunk_create(str.ptr + KENCR_LEN, KAUTH_LEN);
	DBG3(DBG_LIB, "K_encr %B\nK_auth %B", &k_encr, &k_auth);

	if (!this->signer->set_key(this->signer, k_auth) ||
		!this->crypter->set_key(this->crypter, k_encr))
	{
		return FALSE;
	}

	call_hook(this, k_encr, k_auth);

	this->derived = TRUE;
	return TRUE;
}

METHOD(simaka_crypto_t, derive_keys_reauth_msk, bool,
	private_simaka_crypto_t *this, identification_t *id, chunk_t counter,
	chunk_t nonce_s, chunk_t mk, chunk_t *msk)
{
	char xkey[HASH_SIZE_SHA1];
	chunk_t str;
	int i;

	if (!this->hasher->get_hash(this->hasher, id->get_encoding(id), NULL) ||
		!this->hasher->get_hash(this->hasher, counter, NULL) ||
		!this->hasher->get_hash(this->hasher, nonce_s, NULL) ||
		!this->hasher->get_hash(this->hasher, mk, xkey))
	{
		return FALSE;
	}

	/* MSK | EMSK = prf() | prf() | prf() | prf() */
	if (!this->prf->set_key(this->prf, chunk_create(xkey, sizeof(xkey))))
	{
		return FALSE;
	}
	str = chunk_alloca(this->prf->get_block_size(this->prf) * 2);
	for (i = 0; i < 2; i++)
	{
		if (!this->prf->get_bytes(this->prf, chunk_empty,
								  str.ptr + str.len / 2 * i))
		{
			return FALSE;
		}
	}
	*msk = chunk_clone(chunk_create(str.ptr, MSK_LEN));
	DBG3(DBG_LIB, "MSK %B", msk);

	return TRUE;
}

METHOD(simaka_crypto_t, clear_keys, void,
	private_simaka_crypto_t *this)
{
	this->derived = FALSE;
}

METHOD(simaka_crypto_t, destroy, void,
	private_simaka_crypto_t *this)
{
	DESTROY_IF(this->rng);
	DESTROY_IF(this->hasher);
	DESTROY_IF(this->prf);
	DESTROY_IF(this->signer);
	DESTROY_IF(this->crypter);
	free(this);
}

/**
 * See header
 */
simaka_crypto_t *simaka_crypto_create(eap_type_t type)
{
	private_simaka_crypto_t *this;

	INIT(this,
		.public = {
			.get_signer = _get_signer,
			.get_crypter = _get_crypter,
			.get_rng = _get_rng,
			.derive_keys_full = _derive_keys_full,
			.derive_keys_reauth = _derive_keys_reauth,
			.derive_keys_reauth_msk = _derive_keys_reauth_msk,
			.clear_keys = _clear_keys,
			.destroy = _destroy,
		},
		.type = type,
		.rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK),
		.hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1),
		.prf = lib->crypto->create_prf(lib->crypto, PRF_FIPS_SHA1_160),
		.signer = lib->crypto->create_signer(lib->crypto, AUTH_HMAC_SHA1_128),
		.crypter = lib->crypto->create_crypter(lib->crypto, ENCR_AES_CBC, 16),
	);
	if (!this->rng || !this->hasher || !this->prf ||
		!this->signer || !this->crypter)
	{
		DBG1(DBG_LIB, "unable to use %N, missing algorithms",
			 eap_type_names, type);
		destroy(this);
		return NULL;
	}
	return &this->public;
}
