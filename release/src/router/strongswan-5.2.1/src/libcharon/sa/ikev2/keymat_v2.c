/*
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

#include "keymat_v2.h"

#include <daemon.h>
#include <crypto/prf_plus.h>

typedef struct private_keymat_v2_t private_keymat_v2_t;

/**
 * Private data of an keymat_t object.
 */
struct private_keymat_v2_t {

	/**
	 * Public keymat_v2_t interface.
	 */
	keymat_v2_t public;

	/**
	 * IKE_SA Role, initiator or responder
	 */
	bool initiator;

	/**
	 * inbound AEAD
	 */
	aead_t *aead_in;

	/**
	 * outbound AEAD
	 */
	aead_t *aead_out;

	/**
	 * General purpose PRF
	 */
	prf_t *prf;

	/**
	 * Negotiated PRF algorithm
	 */
	pseudo_random_function_t prf_alg;

	/**
	 * Key to derive key material from for CHILD_SAs, rekeying
	 */
	chunk_t skd;

	/**
	 * Key to build outging authentication data (SKp)
	 */
	chunk_t skp_build;

	/**
	 * Key to verify incoming authentication data (SKp)
	 */
	chunk_t skp_verify;
};

METHOD(keymat_t, get_version, ike_version_t,
	private_keymat_v2_t *this)
{
	return IKEV2;
}

METHOD(keymat_t, create_dh, diffie_hellman_t*,
	private_keymat_v2_t *this, diffie_hellman_group_t group)
{
	return lib->crypto->create_dh(lib->crypto, group);
}

METHOD(keymat_t, create_nonce_gen, nonce_gen_t*,
	private_keymat_v2_t *this)
{
	return lib->crypto->create_nonce_gen(lib->crypto);
}

/**
 * Derive IKE keys for a combined AEAD algorithm
 */
static bool derive_ike_aead(private_keymat_v2_t *this, u_int16_t alg,
							u_int16_t key_size, prf_plus_t *prf_plus)
{
	aead_t *aead_i, *aead_r;
	chunk_t key = chunk_empty;
	u_int salt_size;

	switch (alg)
	{
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
			/* RFC 4106 */
			salt_size = 4;
			break;
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
			/* RFC 4309 */
		case ENCR_CAMELLIA_CCM_ICV8:
		case ENCR_CAMELLIA_CCM_ICV12:
		case ENCR_CAMELLIA_CCM_ICV16:
			/* RFC 5529 */
			salt_size = 3;
			break;
		default:
			DBG1(DBG_IKE, "nonce size for %N unknown!",
				 encryption_algorithm_names, alg);
			return FALSE;
	}

	/* SK_ei/SK_er used for encryption */
	aead_i = lib->crypto->create_aead(lib->crypto, alg, key_size / 8, salt_size);
	aead_r = lib->crypto->create_aead(lib->crypto, alg, key_size / 8, salt_size);
	if (aead_i == NULL || aead_r == NULL)
	{
		DBG1(DBG_IKE, "%N %N (key size %d) not supported!",
			 transform_type_names, ENCRYPTION_ALGORITHM,
			 encryption_algorithm_names, alg, key_size);
		goto failure;
	}
	key_size = aead_i->get_key_size(aead_i);
	if (key_size != aead_r->get_key_size(aead_r))
	{
		goto failure;
	}
	if (!prf_plus->allocate_bytes(prf_plus, key_size, &key))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "Sk_ei secret %B", &key);
	if (!aead_i->set_key(aead_i, key))
	{
		goto failure;
	}
	chunk_clear(&key);

	if (!prf_plus->allocate_bytes(prf_plus, key_size, &key))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "Sk_er secret %B", &key);
	if (!aead_r->set_key(aead_r, key))
	{
		goto failure;
	}

	if (this->initiator)
	{
		this->aead_in = aead_r;
		this->aead_out = aead_i;
	}
	else
	{
		this->aead_in = aead_i;
		this->aead_out = aead_r;
	}
	aead_i = aead_r = NULL;

failure:
	DESTROY_IF(aead_i);
	DESTROY_IF(aead_r);
	chunk_clear(&key);
	return this->aead_in && this->aead_out;
}

/**
 * Derive IKE keys for traditional encryption and MAC algorithms
 */
static bool derive_ike_traditional(private_keymat_v2_t *this, u_int16_t enc_alg,
					u_int16_t enc_size, u_int16_t int_alg, prf_plus_t *prf_plus)
{
	crypter_t *crypter_i = NULL, *crypter_r = NULL;
	signer_t *signer_i, *signer_r;
	size_t key_size;
	chunk_t key = chunk_empty;

	signer_i = lib->crypto->create_signer(lib->crypto, int_alg);
	signer_r = lib->crypto->create_signer(lib->crypto, int_alg);
	crypter_i = lib->crypto->create_crypter(lib->crypto, enc_alg, enc_size / 8);
	crypter_r = lib->crypto->create_crypter(lib->crypto, enc_alg, enc_size / 8);
	if (signer_i == NULL || signer_r == NULL)
	{
		DBG1(DBG_IKE, "%N %N not supported!",
			 transform_type_names, INTEGRITY_ALGORITHM,
			 integrity_algorithm_names, int_alg);
		goto failure;
	}
	if (crypter_i == NULL || crypter_r == NULL)
	{
		DBG1(DBG_IKE, "%N %N (key size %d) not supported!",
			 transform_type_names, ENCRYPTION_ALGORITHM,
			 encryption_algorithm_names, enc_alg, enc_size);
		goto failure;
	}

	/* SK_ai/SK_ar used for integrity protection */
	key_size = signer_i->get_key_size(signer_i);

	if (!prf_plus->allocate_bytes(prf_plus, key_size, &key))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "Sk_ai secret %B", &key);
	if (!signer_i->set_key(signer_i, key))
	{
		goto failure;
	}
	chunk_clear(&key);

	if (!prf_plus->allocate_bytes(prf_plus, key_size, &key))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "Sk_ar secret %B", &key);
	if (!signer_r->set_key(signer_r, key))
	{
		goto failure;
	}
	chunk_clear(&key);

	/* SK_ei/SK_er used for encryption */
	key_size = crypter_i->get_key_size(crypter_i);

	if (!prf_plus->allocate_bytes(prf_plus, key_size, &key))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "Sk_ei secret %B", &key);
	if (!crypter_i->set_key(crypter_i, key))
	{
		goto failure;
	}
	chunk_clear(&key);

	if (!prf_plus->allocate_bytes(prf_plus, key_size, &key))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "Sk_er secret %B", &key);
	if (!crypter_r->set_key(crypter_r, key))
	{
		goto failure;
	}

	if (this->initiator)
	{
		this->aead_in = aead_create(crypter_r, signer_r);
		this->aead_out = aead_create(crypter_i, signer_i);
	}
	else
	{
		this->aead_in = aead_create(crypter_i, signer_i);
		this->aead_out = aead_create(crypter_r, signer_r);
	}
	signer_i = signer_r = NULL;
	crypter_i = crypter_r = NULL;

failure:
	chunk_clear(&key);
	DESTROY_IF(signer_i);
	DESTROY_IF(signer_r);
	DESTROY_IF(crypter_i);
	DESTROY_IF(crypter_r);
	return this->aead_in && this->aead_out;
}

METHOD(keymat_v2_t, derive_ike_keys, bool,
	private_keymat_v2_t *this, proposal_t *proposal, diffie_hellman_t *dh,
	chunk_t nonce_i, chunk_t nonce_r, ike_sa_id_t *id,
	pseudo_random_function_t rekey_function, chunk_t rekey_skd)
{
	chunk_t skeyseed, key, secret, full_nonce, fixed_nonce, prf_plus_seed;
	chunk_t spi_i, spi_r;
	prf_plus_t *prf_plus = NULL;
	u_int16_t alg, key_size, int_alg;
	prf_t *rekey_prf = NULL;

	spi_i = chunk_alloca(sizeof(u_int64_t));
	spi_r = chunk_alloca(sizeof(u_int64_t));

	if (dh->get_shared_secret(dh, &secret) != SUCCESS)
	{
		return FALSE;
	}

	/* Create SAs general purpose PRF first, we may use it here */
	if (!proposal->get_algorithm(proposal, PSEUDO_RANDOM_FUNCTION, &alg, NULL))
	{
		DBG1(DBG_IKE, "no %N selected",
			 transform_type_names, PSEUDO_RANDOM_FUNCTION);
		chunk_clear(&secret);
		return FALSE;
	}
	this->prf_alg = alg;
	this->prf = lib->crypto->create_prf(lib->crypto, alg);
	if (this->prf == NULL)
	{
		DBG1(DBG_IKE, "%N %N not supported!",
			 transform_type_names, PSEUDO_RANDOM_FUNCTION,
			 pseudo_random_function_names, alg);
		chunk_clear(&secret);
		return FALSE;
	}
	DBG4(DBG_IKE, "shared Diffie Hellman secret %B", &secret);
	/* full nonce is used as seed for PRF+ ... */
	full_nonce = chunk_cat("cc", nonce_i, nonce_r);
	/* but the PRF may need a fixed key which only uses the first bytes of
	 * the nonces. */
	switch (alg)
	{
		case PRF_AES128_XCBC:
			/* while rfc4434 defines variable keys for AES-XCBC, rfc3664 does
			 * not and therefore fixed key semantics apply to XCBC for key
			 * derivation. */
		case PRF_CAMELLIA128_XCBC:
			/* draft-kanno-ipsecme-camellia-xcbc refers to rfc 4434, we
			 * assume fixed key length. */
			key_size = this->prf->get_key_size(this->prf)/2;
			nonce_i.len = min(nonce_i.len, key_size);
			nonce_r.len = min(nonce_r.len, key_size);
			break;
		default:
			/* all other algorithms use variable key length, full nonce */
			break;
	}
	fixed_nonce = chunk_cat("cc", nonce_i, nonce_r);
	*((u_int64_t*)spi_i.ptr) = id->get_initiator_spi(id);
	*((u_int64_t*)spi_r.ptr) = id->get_responder_spi(id);
	prf_plus_seed = chunk_cat("ccc", full_nonce, spi_i, spi_r);

	/* KEYMAT = prf+ (SKEYSEED, Ni | Nr | SPIi | SPIr)
	 *
	 * if we are rekeying, SKEYSEED is built on another way
	 */
	if (rekey_function == PRF_UNDEFINED) /* not rekeying */
	{
		/* SKEYSEED = prf(Ni | Nr, g^ir) */
		if (this->prf->set_key(this->prf, fixed_nonce) &&
			this->prf->allocate_bytes(this->prf, secret, &skeyseed) &&
			this->prf->set_key(this->prf, skeyseed))
		{
			prf_plus = prf_plus_create(this->prf, TRUE, prf_plus_seed);
		}
	}
	else
	{
		/* SKEYSEED = prf(SK_d (old), [g^ir (new)] | Ni | Nr)
		 * use OLD SAs PRF functions for both prf_plus and prf */
		rekey_prf = lib->crypto->create_prf(lib->crypto, rekey_function);
		if (!rekey_prf)
		{
			DBG1(DBG_IKE, "PRF of old SA %N not supported!",
				 pseudo_random_function_names, rekey_function);
			chunk_clear(&secret);
			chunk_free(&full_nonce);
			chunk_free(&fixed_nonce);
			chunk_clear(&prf_plus_seed);
			return FALSE;
		}
		secret = chunk_cat("mc", secret, full_nonce);
		if (rekey_prf->set_key(rekey_prf, rekey_skd) &&
			rekey_prf->allocate_bytes(rekey_prf, secret, &skeyseed) &&
			rekey_prf->set_key(rekey_prf, skeyseed))
		{
			prf_plus = prf_plus_create(rekey_prf, TRUE, prf_plus_seed);
		}
	}
	DBG4(DBG_IKE, "SKEYSEED %B", &skeyseed);

	chunk_clear(&skeyseed);
	chunk_clear(&secret);
	chunk_free(&full_nonce);
	chunk_free(&fixed_nonce);
	chunk_clear(&prf_plus_seed);

	if (!prf_plus)
	{
		goto failure;
	}

	/* KEYMAT = SK_d | SK_ai | SK_ar | SK_ei | SK_er | SK_pi | SK_pr */

	/* SK_d is used for generating CHILD_SA key mat => store for later use */
	key_size = this->prf->get_key_size(this->prf);
	if (!prf_plus->allocate_bytes(prf_plus, key_size, &this->skd))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "Sk_d secret %B", &this->skd);

	if (!proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM, &alg, &key_size))
	{
		DBG1(DBG_IKE, "no %N selected",
			 transform_type_names, ENCRYPTION_ALGORITHM);
		goto failure;
	}

	if (encryption_algorithm_is_aead(alg))
	{
		if (!derive_ike_aead(this, alg, key_size, prf_plus))
		{
			goto failure;
		}
	}
	else
	{
		if (!proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM,
									 &int_alg, NULL))
		{
			DBG1(DBG_IKE, "no %N selected",
				 transform_type_names, INTEGRITY_ALGORITHM);
			goto failure;
		}
		if (!derive_ike_traditional(this, alg, key_size, int_alg, prf_plus))
		{
			goto failure;
		}
	}

	/* SK_pi/SK_pr used for authentication => stored for later */
	key_size = this->prf->get_key_size(this->prf);
	if (!prf_plus->allocate_bytes(prf_plus, key_size, &key))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "Sk_pi secret %B", &key);
	if (this->initiator)
	{
		this->skp_build = key;
	}
	else
	{
		this->skp_verify = key;
	}
	if (!prf_plus->allocate_bytes(prf_plus, key_size, &key))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "Sk_pr secret %B", &key);
	if (this->initiator)
	{
		this->skp_verify = key;
	}
	else
	{
		this->skp_build = key;
	}

	/* all done, prf_plus not needed anymore */
failure:
	DESTROY_IF(prf_plus);
	DESTROY_IF(rekey_prf);

	return this->skp_build.len && this->skp_verify.len;
}

METHOD(keymat_v2_t, derive_child_keys, bool,
	private_keymat_v2_t *this, proposal_t *proposal, diffie_hellman_t *dh,
	chunk_t nonce_i, chunk_t nonce_r, chunk_t *encr_i, chunk_t *integ_i,
	chunk_t *encr_r, chunk_t *integ_r)
{
	u_int16_t enc_alg, int_alg, enc_size = 0, int_size = 0;
	chunk_t seed, secret = chunk_empty;
	prf_plus_t *prf_plus;

	if (proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM,
								&enc_alg, &enc_size))
	{
		DBG2(DBG_CHD, "  using %N for encryption",
			 encryption_algorithm_names, enc_alg);

		if (!enc_size)
		{
			enc_size = keymat_get_keylen_encr(enc_alg);
		}
		if (enc_alg != ENCR_NULL && !enc_size)
		{
			DBG1(DBG_CHD, "no keylength defined for %N",
				 encryption_algorithm_names, enc_alg);
			return FALSE;
		}
		/* to bytes */
		enc_size /= 8;

		/* CCM/GCM/CTR/GMAC needs additional bytes */
		switch (enc_alg)
		{
			case ENCR_AES_CCM_ICV8:
			case ENCR_AES_CCM_ICV12:
			case ENCR_AES_CCM_ICV16:
			case ENCR_CAMELLIA_CCM_ICV8:
			case ENCR_CAMELLIA_CCM_ICV12:
			case ENCR_CAMELLIA_CCM_ICV16:
				enc_size += 3;
				break;
			case ENCR_AES_GCM_ICV8:
			case ENCR_AES_GCM_ICV12:
			case ENCR_AES_GCM_ICV16:
			case ENCR_AES_CTR:
			case ENCR_NULL_AUTH_AES_GMAC:
				enc_size += 4;
				break;
			default:
				break;
		}
	}

	if (proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM,
								&int_alg, &int_size))
	{
		DBG2(DBG_CHD, "  using %N for integrity",
			 integrity_algorithm_names, int_alg);

		if (!int_size)
		{
			int_size = keymat_get_keylen_integ(int_alg);
		}
		if (!int_size)
		{
			DBG1(DBG_CHD, "no keylength defined for %N",
				 integrity_algorithm_names, int_alg);
			return FALSE;
		}
		/* to bytes */
		int_size /= 8;
	}

	if (!this->prf->set_key(this->prf, this->skd))
	{
		return FALSE;
	}

	if (dh)
	{
		if (dh->get_shared_secret(dh, &secret) != SUCCESS)
		{
			return FALSE;
		}
		DBG4(DBG_CHD, "DH secret %B", &secret);
	}
	seed = chunk_cata("scc", secret, nonce_i, nonce_r);
	DBG4(DBG_CHD, "seed %B", &seed);

	prf_plus = prf_plus_create(this->prf, TRUE, seed);
	memwipe(seed.ptr, seed.len);

	if (!prf_plus)
	{
		return FALSE;
	}

	*encr_i = *integ_i = *encr_r = *integ_r = chunk_empty;
	if (!prf_plus->allocate_bytes(prf_plus, enc_size, encr_i) ||
		!prf_plus->allocate_bytes(prf_plus, int_size, integ_i) ||
		!prf_plus->allocate_bytes(prf_plus, enc_size, encr_r) ||
		!prf_plus->allocate_bytes(prf_plus, int_size, integ_r))
	{
		chunk_free(encr_i);
		chunk_free(integ_i);
		chunk_free(encr_r);
		chunk_free(integ_r);
		prf_plus->destroy(prf_plus);
		return FALSE;
	}

	prf_plus->destroy(prf_plus);

	if (enc_size)
	{
		DBG4(DBG_CHD, "encryption initiator key %B", encr_i);
		DBG4(DBG_CHD, "encryption responder key %B", encr_r);
	}
	if (int_size)
	{
		DBG4(DBG_CHD, "integrity initiator key %B", integ_i);
		DBG4(DBG_CHD, "integrity responder key %B", integ_r);
	}
	return TRUE;
}

METHOD(keymat_v2_t, get_skd, pseudo_random_function_t,
	private_keymat_v2_t *this, chunk_t *skd)
{
	*skd = this->skd;
	return this->prf_alg;
}

METHOD(keymat_t, get_aead, aead_t*,
	private_keymat_v2_t *this, bool in)
{
	return in ? this->aead_in : this->aead_out;
}

METHOD(keymat_v2_t, get_auth_octets, bool,
	private_keymat_v2_t *this, bool verify, chunk_t ike_sa_init,
	chunk_t nonce, identification_t *id, char reserved[3], chunk_t *octets)
{
	chunk_t chunk, idx;
	chunk_t skp;

	skp = verify ? this->skp_verify : this->skp_build;

	chunk = chunk_alloca(4);
	chunk.ptr[0] = id->get_type(id);
	memcpy(chunk.ptr + 1, reserved, 3);
	idx = chunk_cata("cc", chunk, id->get_encoding(id));

	DBG3(DBG_IKE, "IDx' %B", &idx);
	DBG4(DBG_IKE, "SK_p %B", &skp);
	if (!this->prf->set_key(this->prf, skp) ||
		!this->prf->allocate_bytes(this->prf, idx, &chunk))
	{
		return FALSE;
	}
	*octets = chunk_cat("ccm", ike_sa_init, nonce, chunk);
	DBG3(DBG_IKE, "octets = message + nonce + prf(Sk_px, IDx') %B", octets);
	return TRUE;
}

/**
 * Key pad for the AUTH method SHARED_KEY_MESSAGE_INTEGRITY_CODE.
 */
#define IKEV2_KEY_PAD "Key Pad for IKEv2"
#define IKEV2_KEY_PAD_LENGTH 17

METHOD(keymat_v2_t, get_psk_sig, bool,
	private_keymat_v2_t *this, bool verify, chunk_t ike_sa_init, chunk_t nonce,
	chunk_t secret, identification_t *id, char reserved[3], chunk_t *sig)
{
	chunk_t key_pad, key, octets;

	if (!secret.len)
	{	/* EAP uses SK_p if no MSK has been established */
		secret = verify ? this->skp_verify : this->skp_build;
	}
	if (!get_auth_octets(this, verify, ike_sa_init, nonce, id, reserved, &octets))
	{
		return FALSE;
	}
	/* AUTH = prf(prf(Shared Secret,"Key Pad for IKEv2"), <msg octets>) */
	key_pad = chunk_create(IKEV2_KEY_PAD, IKEV2_KEY_PAD_LENGTH);
	if (!this->prf->set_key(this->prf, secret) ||
		!this->prf->allocate_bytes(this->prf, key_pad, &key))
	{
		chunk_free(&octets);
		return FALSE;
	}
	if (!this->prf->set_key(this->prf, key) ||
		!this->prf->allocate_bytes(this->prf, octets, sig))
	{
		chunk_free(&key);
		chunk_free(&octets);
		return FALSE;
	}
	DBG4(DBG_IKE, "secret %B", &secret);
	DBG4(DBG_IKE, "prf(secret, keypad) %B", &key);
	DBG3(DBG_IKE, "AUTH = prf(prf(secret, keypad), octets) %B", sig);
	chunk_free(&octets);
	chunk_free(&key);

	return TRUE;
}

METHOD(keymat_t, destroy, void,
	private_keymat_v2_t *this)
{
	DESTROY_IF(this->aead_in);
	DESTROY_IF(this->aead_out);
	DESTROY_IF(this->prf);
	chunk_clear(&this->skd);
	chunk_clear(&this->skp_verify);
	chunk_clear(&this->skp_build);
	free(this);
}

/**
 * See header
 */
keymat_v2_t *keymat_v2_create(bool initiator)
{
	private_keymat_v2_t *this;

	INIT(this,
		.public = {
			.keymat = {
				.get_version = _get_version,
				.create_dh = _create_dh,
				.create_nonce_gen = _create_nonce_gen,
				.get_aead = _get_aead,
				.destroy = _destroy,
			},
			.derive_ike_keys = _derive_ike_keys,
			.derive_child_keys = _derive_child_keys,
			.get_skd = _get_skd,
			.get_auth_octets = _get_auth_octets,
			.get_psk_sig = _get_psk_sig,
		},
		.initiator = initiator,
		.prf_alg = PRF_UNDEFINED,
	);

	return &this->public;
}
