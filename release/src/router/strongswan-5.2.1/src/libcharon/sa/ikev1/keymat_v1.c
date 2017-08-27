/*
 * Copyright (C) 2011 Tobias Brunner
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

#include "keymat_v1.h"

#include <daemon.h>
#include <encoding/generator.h>
#include <encoding/payloads/nonce_payload.h>
#include <collections/linked_list.h>

typedef struct private_keymat_v1_t private_keymat_v1_t;

/**
 * Max. number of IVs to track.
 */
#define MAX_IV 3

/**
 * Max. number of Quick Modes to track.
 */
#define MAX_QM 2

/**
 * Data stored for IVs
 */
typedef struct {
	/** message ID */
	u_int32_t mid;
	/** current IV */
	chunk_t iv;
	/** last block of encrypted message */
	chunk_t last_block;
} iv_data_t;

/**
 * Private data of an keymat_t object.
 */
struct private_keymat_v1_t {

	/**
	 * Public keymat_v1_t interface.
	 */
	keymat_v1_t public;

	/**
	 * IKE_SA Role, initiator or responder
	 */
	bool initiator;

	/**
	 * General purpose PRF
	 */
	prf_t *prf;

	/**
	 * PRF to create Phase 1 HASH payloads
	 */
	prf_t *prf_auth;

	/**
	 * Crypter wrapped in an aead_t interface
	 */
	aead_t *aead;

	/**
	 * Hasher used for IV generation (and other things like e.g. NAT-T)
	 */
	hasher_t *hasher;

	/**
	 * Key used for authentication during main mode
	 */
	chunk_t skeyid;

	/**
	 * Key to derive key material from for non-ISAKMP SAs, rekeying
	 */
	chunk_t skeyid_d;

	/**
	 * Key used for authentication after main mode
	 */
	chunk_t skeyid_a;

	/**
	 * Phase 1 IV
	 */
	iv_data_t phase1_iv;

	/**
	 * Keep track of IVs for exchanges after phase 1. We store only a limited
	 * number of IVs in an MRU sort of way. Stores iv_data_t objects.
	 */
	linked_list_t *ivs;

	/**
	 * Keep track of Nonces during Quick Mode exchanges. Only a limited number
	 * of QMs are tracked at the same time. Stores qm_data_t objects.
	 */
	linked_list_t *qms;
};


/**
 * Destroy an iv_data_t object.
 */
static void iv_data_destroy(iv_data_t *this)
{
	chunk_free(&this->last_block);
	chunk_free(&this->iv);
	free(this);
}

/**
 * Data stored for Quick Mode exchanges
 */
typedef struct {
	/** message ID */
	u_int32_t mid;
	/** Ni_b (Nonce from first message) */
	chunk_t n_i;
	/** Nr_b (Nonce from second message) */
	chunk_t n_r;
} qm_data_t;

/**
 * Destroy a qm_data_t object.
 */
static void qm_data_destroy(qm_data_t *this)
{
	chunk_free(&this->n_i);
	chunk_free(&this->n_r);
	free(this);
}

/**
 * Constants used in key derivation.
 */
static const chunk_t octet_0 = chunk_from_chars(0x00);
static const chunk_t octet_1 = chunk_from_chars(0x01);
static const chunk_t octet_2 = chunk_from_chars(0x02);

/**
 * Simple aead_t implementation without support for authentication.
 */
typedef struct {
	/** implements aead_t interface */
	aead_t aead;
	/** crypter to be used */
	crypter_t *crypter;
} private_aead_t;


METHOD(aead_t, encrypt, bool,
	private_aead_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
	chunk_t *encrypted)
{
	return this->crypter->encrypt(this->crypter, plain, iv, encrypted);
}

METHOD(aead_t, decrypt, bool,
	private_aead_t *this, chunk_t encrypted, chunk_t assoc, chunk_t iv,
	chunk_t *plain)
{
	return this->crypter->decrypt(this->crypter, encrypted, iv, plain);
}

METHOD(aead_t, get_block_size, size_t,
	private_aead_t *this)
{
	return this->crypter->get_block_size(this->crypter);
}

METHOD(aead_t, get_icv_size, size_t,
	private_aead_t *this)
{
	return 0;
}

METHOD(aead_t, get_iv_size, size_t,
	private_aead_t *this)
{
	/* in order to create the messages properly we return 0 here */
	return 0;
}

METHOD(aead_t, get_iv_gen, iv_gen_t*,
	private_aead_t *this)
{
	/* IVs are retrieved via keymat_v1.get_iv() */
	return NULL;
}

METHOD(aead_t, get_key_size, size_t,
	private_aead_t *this)
{
	return this->crypter->get_key_size(this->crypter);
}

METHOD(aead_t, set_key, bool,
	private_aead_t *this, chunk_t key)
{
	return this->crypter->set_key(this->crypter, key);
}

METHOD(aead_t, aead_destroy, void,
	private_aead_t *this)
{
	this->crypter->destroy(this->crypter);
	free(this);
}

/**
 * Expand SKEYID_e according to Appendix B in RFC 2409.
 * TODO-IKEv1: verify keys (e.g. for weak keys, see Appendix B)
 */
static bool expand_skeyid_e(chunk_t skeyid_e, size_t key_size, prf_t *prf,
							chunk_t *ka)
{
	size_t block_size;
	chunk_t seed;
	int i;

	if (skeyid_e.len >= key_size)
	{	/* no expansion required, reduce to key_size */
		skeyid_e.len = key_size;
		*ka = skeyid_e;
		return TRUE;
	}
	block_size = prf->get_block_size(prf);
	*ka = chunk_alloc((key_size / block_size + 1) * block_size);
	ka->len = key_size;

	/* Ka = K1 | K2 | ..., K1 = prf(SKEYID_e, 0), K2 = prf(SKEYID_e, K1) ... */
	if (!prf->set_key(prf, skeyid_e))
	{
		chunk_clear(ka);
		chunk_clear(&skeyid_e);
		return FALSE;
	}
	seed = octet_0;
	for (i = 0; i < key_size; i += block_size)
	{
		if (!prf->get_bytes(prf, seed, ka->ptr + i))
		{
			chunk_clear(ka);
			chunk_clear(&skeyid_e);
			return FALSE;
		}
		seed = chunk_create(ka->ptr + i, block_size);
	}
	chunk_clear(&skeyid_e);
	return TRUE;
}

/**
 * Create a simple implementation of the aead_t interface which only encrypts
 * or decrypts data.
 */
static aead_t *create_aead(proposal_t *proposal, prf_t *prf, chunk_t skeyid_e)
{
	private_aead_t *this;
	u_int16_t alg, key_size;
	crypter_t *crypter;
	chunk_t ka;

	if (!proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM, &alg,
								 &key_size))
	{
		DBG1(DBG_IKE, "no %N selected",
			 transform_type_names, ENCRYPTION_ALGORITHM);
		return NULL;
	}
	crypter = lib->crypto->create_crypter(lib->crypto, alg, key_size / 8);
	if (!crypter)
	{
		DBG1(DBG_IKE, "%N %N (key size %d) not supported!",
			 transform_type_names, ENCRYPTION_ALGORITHM,
			 encryption_algorithm_names, alg, key_size);
		return NULL;
	}
	key_size = crypter->get_key_size(crypter);
	if (!expand_skeyid_e(skeyid_e, crypter->get_key_size(crypter), prf, &ka))
	{
		return NULL;
	}
	DBG4(DBG_IKE, "encryption key Ka %B", &ka);
	if (!crypter->set_key(crypter, ka))
	{
		chunk_clear(&ka);
		return NULL;
	}
	chunk_clear(&ka);

	INIT(this,
		.aead = {
			.encrypt = _encrypt,
			.decrypt = _decrypt,
			.get_block_size = _get_block_size,
			.get_icv_size = _get_icv_size,
			.get_iv_size = _get_iv_size,
			.get_iv_gen = _get_iv_gen,
			.get_key_size = _get_key_size,
			.set_key = _set_key,
			.destroy = _aead_destroy,
		},
		.crypter = crypter,
	);
	return &this->aead;
}

/**
 * Converts integrity algorithm to PRF algorithm
 */
static u_int16_t auth_to_prf(u_int16_t alg)
{
	switch (alg)
	{
		case AUTH_HMAC_SHA1_96:
			return PRF_HMAC_SHA1;
		case AUTH_HMAC_SHA2_256_128:
			return PRF_HMAC_SHA2_256;
		case AUTH_HMAC_SHA2_384_192:
			return PRF_HMAC_SHA2_384;
		case AUTH_HMAC_SHA2_512_256:
			return PRF_HMAC_SHA2_512;
		case AUTH_HMAC_MD5_96:
			return PRF_HMAC_MD5;
		case AUTH_AES_XCBC_96:
			return PRF_AES128_XCBC;
		default:
			return PRF_UNDEFINED;
	}
}

/**
 * Converts integrity algorithm to hash algorithm
 */
static u_int16_t auth_to_hash(u_int16_t alg)
{
	switch (alg)
	{
		case AUTH_HMAC_SHA1_96:
			return HASH_SHA1;
		case AUTH_HMAC_SHA2_256_128:
			return HASH_SHA256;
		case AUTH_HMAC_SHA2_384_192:
			return HASH_SHA384;
		case AUTH_HMAC_SHA2_512_256:
			return HASH_SHA512;
		case AUTH_HMAC_MD5_96:
			return HASH_MD5;
		default:
			return HASH_UNKNOWN;
	}
}

/**
 * Adjust the key length for PRF algorithms that expect a fixed key length.
 */
static void adjust_keylen(u_int16_t alg, chunk_t *key)
{
	switch (alg)
	{
		case PRF_AES128_XCBC:
			/* while rfc4434 defines variable keys for AES-XCBC, rfc3664 does
			 * not and therefore fixed key semantics apply to XCBC for key
			 * derivation. */
			key->len = min(key->len, 16);
			break;
		default:
			/* all other algorithms use variable key length */
			break;
	}
}

METHOD(keymat_v1_t, derive_ike_keys, bool,
	private_keymat_v1_t *this, proposal_t *proposal, diffie_hellman_t *dh,
	chunk_t dh_other, chunk_t nonce_i, chunk_t nonce_r, ike_sa_id_t *id,
	auth_method_t auth, shared_key_t *shared_key)
{
	chunk_t g_xy, g_xi, g_xr, dh_me, spi_i, spi_r, nonces, data, skeyid_e;
	chunk_t skeyid;
	u_int16_t alg;

	spi_i = chunk_alloca(sizeof(u_int64_t));
	spi_r = chunk_alloca(sizeof(u_int64_t));

	if (!proposal->get_algorithm(proposal, PSEUDO_RANDOM_FUNCTION, &alg, NULL))
	{	/* no PRF negotiated, use HMAC version of integrity algorithm instead */
		if (!proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM, &alg, NULL)
			|| (alg = auth_to_prf(alg)) == PRF_UNDEFINED)
		{
			DBG1(DBG_IKE, "no %N selected",
				 transform_type_names, PSEUDO_RANDOM_FUNCTION);
			return FALSE;
		}
	}
	this->prf = lib->crypto->create_prf(lib->crypto, alg);
	if (!this->prf)
	{
		DBG1(DBG_IKE, "%N %N not supported!",
			 transform_type_names, PSEUDO_RANDOM_FUNCTION,
			 pseudo_random_function_names, alg);
		return FALSE;
	}
	if (this->prf->get_block_size(this->prf) <
		this->prf->get_key_size(this->prf))
	{	/* TODO-IKEv1: support PRF output expansion (RFC 2409, Appendix B) */
		DBG1(DBG_IKE, "expansion of %N %N output not supported!",
			 transform_type_names, PSEUDO_RANDOM_FUNCTION,
			 pseudo_random_function_names, alg);
		return FALSE;
	}

	if (dh->get_shared_secret(dh, &g_xy) != SUCCESS)
	{
		return FALSE;
	}
	DBG4(DBG_IKE, "shared Diffie Hellman secret %B", &g_xy);

	*((u_int64_t*)spi_i.ptr) = id->get_initiator_spi(id);
	*((u_int64_t*)spi_r.ptr) = id->get_responder_spi(id);
	nonces = chunk_cata("cc", nonce_i, nonce_r);

	switch (auth)
	{
		case AUTH_PSK:
		case AUTH_XAUTH_INIT_PSK:
		case AUTH_XAUTH_RESP_PSK:
		{	/* SKEYID = prf(pre-shared-key, Ni_b | Nr_b) */
			chunk_t psk;
			if (!shared_key)
			{
				chunk_clear(&g_xy);
				return FALSE;
			}
			psk = shared_key->get_key(shared_key);
			adjust_keylen(alg, &psk);
			if (!this->prf->set_key(this->prf, psk) ||
				!this->prf->allocate_bytes(this->prf, nonces, &skeyid))
			{
				chunk_clear(&g_xy);
				return FALSE;
			}
			break;
		}
		case AUTH_RSA:
		case AUTH_ECDSA_256:
		case AUTH_ECDSA_384:
		case AUTH_ECDSA_521:
		case AUTH_XAUTH_INIT_RSA:
		case AUTH_XAUTH_RESP_RSA:
		case AUTH_HYBRID_INIT_RSA:
		case AUTH_HYBRID_RESP_RSA:
		{
			if (!this->prf->set_key(this->prf, nonces) ||
				!this->prf->allocate_bytes(this->prf, g_xy, &skeyid))
			{
				chunk_clear(&g_xy);
				return FALSE;
			}
			break;
		}
		default:
			/* TODO-IKEv1: implement key derivation for other schemes */
			/* authentication class not supported */
			chunk_clear(&g_xy);
			return FALSE;
	}
	adjust_keylen(alg, &skeyid);
	DBG4(DBG_IKE, "SKEYID %B", &skeyid);

	/* SKEYID_d = prf(SKEYID, g^xy | CKY-I | CKY-R | 0) */
	data = chunk_cat("cccc", g_xy, spi_i, spi_r, octet_0);
	if (!this->prf->set_key(this->prf, skeyid) ||
		!this->prf->allocate_bytes(this->prf, data, &this->skeyid_d))
	{
		chunk_clear(&g_xy);
		chunk_clear(&data);
		return FALSE;
	}
	chunk_clear(&data);
	DBG4(DBG_IKE, "SKEYID_d %B", &this->skeyid_d);

	/* SKEYID_a = prf(SKEYID, SKEYID_d | g^xy | CKY-I | CKY-R | 1) */
	data = chunk_cat("ccccc", this->skeyid_d, g_xy, spi_i, spi_r, octet_1);
	if (!this->prf->allocate_bytes(this->prf, data, &this->skeyid_a))
	{
		chunk_clear(&g_xy);
		chunk_clear(&data);
		return FALSE;
	}
	chunk_clear(&data);
	DBG4(DBG_IKE, "SKEYID_a %B", &this->skeyid_a);

	/* SKEYID_e = prf(SKEYID, SKEYID_a | g^xy | CKY-I | CKY-R | 2) */
	data = chunk_cat("ccccc", this->skeyid_a, g_xy, spi_i, spi_r, octet_2);
	if (!this->prf->allocate_bytes(this->prf, data, &skeyid_e))
	{
		chunk_clear(&g_xy);
		chunk_clear(&data);
		return FALSE;
	}
	chunk_clear(&data);
	DBG4(DBG_IKE, "SKEYID_e %B", &skeyid_e);

	chunk_clear(&g_xy);

	switch (auth)
	{
		case AUTH_ECDSA_256:
			alg = PRF_HMAC_SHA2_256;
			break;
		case AUTH_ECDSA_384:
			alg =  PRF_HMAC_SHA2_384;
			break;
		case AUTH_ECDSA_521:
			alg = PRF_HMAC_SHA2_512;
			break;
		default:
			/* use proposal algorithm */
			break;
	}
	this->prf_auth = lib->crypto->create_prf(lib->crypto, alg);
	if (!this->prf_auth)
	{
		DBG1(DBG_IKE, "%N %N not supported!",
			 transform_type_names, PSEUDO_RANDOM_FUNCTION,
			 pseudo_random_function_names, alg);
		chunk_clear(&skeyid);
		return FALSE;
	}
	if (!this->prf_auth->set_key(this->prf_auth, skeyid))
	{
		chunk_clear(&skeyid);
		return FALSE;
	}
	chunk_clear(&skeyid);

	this->aead = create_aead(proposal, this->prf, skeyid_e);
	if (!this->aead)
	{
		return FALSE;
	}
	if (!this->hasher && !this->public.create_hasher(&this->public, proposal))
	{
		return FALSE;
	}

	dh->get_my_public_value(dh, &dh_me);
	g_xi = this->initiator ? dh_me : dh_other;
	g_xr = this->initiator ? dh_other : dh_me;

	/* initial IV = hash(g^xi | g^xr) */
	data = chunk_cata("cc", g_xi, g_xr);
	chunk_free(&dh_me);
	if (!this->hasher->allocate_hash(this->hasher, data, &this->phase1_iv.iv))
	{
		return FALSE;
	}
	if (this->phase1_iv.iv.len > this->aead->get_block_size(this->aead))
	{
		this->phase1_iv.iv.len = this->aead->get_block_size(this->aead);
	}
	DBG4(DBG_IKE, "initial IV %B", &this->phase1_iv.iv);

	return TRUE;
}

METHOD(keymat_v1_t, derive_child_keys, bool,
	private_keymat_v1_t *this, proposal_t *proposal, diffie_hellman_t *dh,
	u_int32_t spi_i, u_int32_t spi_r, chunk_t nonce_i, chunk_t nonce_r,
	chunk_t *encr_i, chunk_t *integ_i, chunk_t *encr_r, chunk_t *integ_r)
{
	u_int16_t enc_alg, int_alg, enc_size = 0, int_size = 0;
	u_int8_t protocol;
	prf_plus_t *prf_plus;
	chunk_t seed, secret = chunk_empty;
	bool success = FALSE;

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

	/* KEYMAT = prf+(SKEYID_d, [ g(qm)^xy | ] protocol | SPI | Ni_b | Nr_b) */
	if (!this->prf->set_key(this->prf, this->skeyid_d))
	{
		return FALSE;
	}
	protocol = proposal->get_protocol(proposal);
	if (dh)
	{
		if (dh->get_shared_secret(dh, &secret) != SUCCESS)
		{
			return FALSE;
		}
		DBG4(DBG_CHD, "DH secret %B", &secret);
	}

	*encr_r = *integ_r = *encr_i = *integ_i = chunk_empty;
	seed = chunk_cata("ccccc", secret, chunk_from_thing(protocol),
					  chunk_from_thing(spi_r), nonce_i, nonce_r);
	DBG4(DBG_CHD, "initiator SA seed %B", &seed);

	prf_plus = prf_plus_create(this->prf, FALSE, seed);
	if (!prf_plus ||
		!prf_plus->allocate_bytes(prf_plus, enc_size, encr_i) ||
		!prf_plus->allocate_bytes(prf_plus, int_size, integ_i))
	{
		goto failure;
	}

	seed = chunk_cata("ccccc", secret, chunk_from_thing(protocol),
					  chunk_from_thing(spi_i), nonce_i, nonce_r);
	DBG4(DBG_CHD, "responder SA seed %B", &seed);
	prf_plus->destroy(prf_plus);
	prf_plus = prf_plus_create(this->prf, FALSE, seed);
	if (!prf_plus ||
		!prf_plus->allocate_bytes(prf_plus, enc_size, encr_r) ||
		!prf_plus->allocate_bytes(prf_plus, int_size, integ_r))
	{
		goto failure;
	}

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
	success = TRUE;

failure:
	if (!success)
	{
		chunk_clear(encr_i);
		chunk_clear(integ_i);
		chunk_clear(encr_r);
		chunk_clear(integ_r);
	}
	DESTROY_IF(prf_plus);
	chunk_clear(&secret);

	return success;
}

METHOD(keymat_v1_t, create_hasher, bool,
	private_keymat_v1_t *this, proposal_t *proposal)
{
	u_int16_t alg;
	if (!proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM, &alg, NULL) ||
		(alg = auth_to_hash(alg)) == HASH_UNKNOWN)
	{
		DBG1(DBG_IKE, "no %N selected", transform_type_names, HASH_ALGORITHM);
		return FALSE;
	}
	this->hasher = lib->crypto->create_hasher(lib->crypto, alg);
	if (!this->hasher)
	{
		DBG1(DBG_IKE, "%N %N not supported!",
			 transform_type_names, HASH_ALGORITHM,
			 hash_algorithm_names, alg);
		return FALSE;
	}
	return TRUE;
}

METHOD(keymat_v1_t, get_hasher, hasher_t*,
	private_keymat_v1_t *this)
{
	return this->hasher;
}

METHOD(keymat_v1_t, get_hash, bool,
	private_keymat_v1_t *this, bool initiator, chunk_t dh, chunk_t dh_other,
	ike_sa_id_t *ike_sa_id, chunk_t sa_i, chunk_t id, chunk_t *hash)
{
	chunk_t data;
	u_int64_t spi, spi_other;

	/* HASH_I = prf(SKEYID, g^xi | g^xr | CKY-I | CKY-R | SAi_b | IDii_b )
	 * HASH_R = prf(SKEYID, g^xr | g^xi | CKY-R | CKY-I | SAi_b | IDir_b )
	 */
	if (initiator)
	{
		spi = ike_sa_id->get_initiator_spi(ike_sa_id);
		spi_other = ike_sa_id->get_responder_spi(ike_sa_id);
	}
	else
	{
		spi_other = ike_sa_id->get_initiator_spi(ike_sa_id);
		spi = ike_sa_id->get_responder_spi(ike_sa_id);
	}
	data = chunk_cat("cccccc", dh, dh_other,
					 chunk_from_thing(spi), chunk_from_thing(spi_other),
					 sa_i, id);

	DBG3(DBG_IKE, "HASH_%c data %B", initiator ? 'I' : 'R', &data);

	if (!this->prf_auth->allocate_bytes(this->prf_auth, data, hash))
	{
		free(data.ptr);
		return FALSE;
	}

	DBG3(DBG_IKE, "HASH_%c %B", initiator ? 'I' : 'R', hash);

	free(data.ptr);
	return TRUE;
}

/**
 * Get the nonce value found in the given message.
 * Returns FALSE if none is found.
 */
static bool get_nonce(message_t *message, chunk_t *n)
{
	nonce_payload_t *nonce;
	nonce = (nonce_payload_t*)message->get_payload(message, PLV1_NONCE);
	if (nonce)
	{
		*n = nonce->get_nonce(nonce);
		return TRUE;
	}
	return FALSE;
}

/**
 * Generate the message data in order to generate the hashes.
 */
static chunk_t get_message_data(message_t *message, generator_t *generator)
{
	payload_t *payload, *next;
	enumerator_t *enumerator;
	u_int32_t *lenpos;

	if (message->is_encoded(message))
	{	/* inbound, although the message is generated, we cannot access the
		 * cleartext message data, so generate it anyway */
		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			if (payload->get_type(payload) == PLV1_HASH)
			{
				continue;
			}
			generator->generate_payload(generator, payload);
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		/* outbound, generate the payloads (there is no HASH payload yet) */
		enumerator = message->create_payload_enumerator(message);
		if (enumerator->enumerate(enumerator, &payload))
		{
			while (enumerator->enumerate(enumerator, &next))
			{
				payload->set_next_type(payload, next->get_type(next));
				generator->generate_payload(generator, payload);
				payload = next;
			}
			payload->set_next_type(payload, PL_NONE);
			generator->generate_payload(generator, payload);
		}
		enumerator->destroy(enumerator);
	}
	return generator->get_chunk(generator, &lenpos);
}

/**
 * Try to find data about a Quick Mode with the given message ID,
 * if none is found, state is generated.
 */
static qm_data_t *lookup_quick_mode(private_keymat_v1_t *this, u_int32_t mid)
{
	enumerator_t *enumerator;
	qm_data_t *qm, *found = NULL;

	enumerator = this->qms->create_enumerator(this->qms);
	while (enumerator->enumerate(enumerator, &qm))
	{
		if (qm->mid == mid)
		{	/* state gets moved to the front of the list */
			this->qms->remove_at(this->qms, enumerator);
			found = qm;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!found)
	{
		INIT(found,
			.mid = mid,
		);
	}
	this->qms->insert_first(this->qms, found);
	/* remove least recently used state if maximum reached */
	if (this->qms->get_count(this->qms) > MAX_QM &&
		this->qms->remove_last(this->qms, (void**)&qm) == SUCCESS)
	{
		qm_data_destroy(qm);
	}
	return found;
}

METHOD(keymat_v1_t, get_hash_phase2, bool,
	private_keymat_v1_t *this, message_t *message, chunk_t *hash)
{
	u_int32_t mid, mid_n;
	chunk_t data = chunk_empty;
	bool add_message = TRUE;
	char *name = "Hash";

	if (!this->prf)
	{	/* no keys derived yet */
		return FALSE;
	}

	mid = message->get_message_id(message);
	mid_n = htonl(mid);

	/* Hashes are simple for most exchanges in Phase 2:
	 *   Hash = prf(SKEYID_a, M-ID | Complete message after HASH payload)
	 * For Quick Mode there are three hashes:
	 *   Hash(1) = same as above
	 *   Hash(2) = prf(SKEYID_a, M-ID | Ni_b | Message after HASH payload)
	 *   Hash(3) = prf(SKEYID_a, 0 | M-ID | Ni_b | Nr_b)
	 * So, for Quick Mode we keep track of the nonce values.
	 */
	switch (message->get_exchange_type(message))
	{
		case QUICK_MODE:
		{
			qm_data_t *qm = lookup_quick_mode(this, mid);
			if (!qm->n_i.ptr)
			{	/* Hash(1) = prf(SKEYID_a, M-ID | Message after HASH payload) */
				name = "Hash(1)";
				if (!get_nonce(message, &qm->n_i))
				{
					return FALSE;
				}
				data = chunk_from_thing(mid_n);
			}
			else if (!qm->n_r.ptr)
			{	/* Hash(2) = prf(SKEYID_a, M-ID | Ni_b | Message after HASH) */
				name = "Hash(2)";
				if (!get_nonce(message, &qm->n_r))
				{
					return FALSE;
				}
				data = chunk_cata("cc", chunk_from_thing(mid_n), qm->n_i);
			}
			else
			{	/* Hash(3) = prf(SKEYID_a, 0 | M-ID | Ni_b | Nr_b) */
				name = "Hash(3)";
				data = chunk_cata("cccc", octet_0, chunk_from_thing(mid_n),
								  qm->n_i, qm->n_r);
				add_message = FALSE;
				/* we don't need the state anymore */
				this->qms->remove(this->qms, qm, NULL);
				qm_data_destroy(qm);
			}
			break;
		}
		case TRANSACTION:
		case INFORMATIONAL_V1:
			/* Hash = prf(SKEYID_a, M-ID | Message after HASH payload) */
			data = chunk_from_thing(mid_n);
			break;
		default:
			return FALSE;
	}
	if (!this->prf->set_key(this->prf, this->skeyid_a))
	{
		return FALSE;
	}
	if (add_message)
	{
		generator_t *generator;
		chunk_t msg;

		generator = generator_create_no_dbg();
		msg = get_message_data(message, generator);
		if (!this->prf->allocate_bytes(this->prf, data, NULL) ||
			!this->prf->allocate_bytes(this->prf, msg, hash))
		{
			generator->destroy(generator);
			return FALSE;
		}
		generator->destroy(generator);
	}
	else
	{
		if (!this->prf->allocate_bytes(this->prf, data, hash))
		{
			return FALSE;
		}
	}
	DBG3(DBG_IKE, "%s %B", name, hash);
	return TRUE;
}

/**
 * Generate an IV
 */
static bool generate_iv(private_keymat_v1_t *this, iv_data_t *iv)
{
	if (iv->mid == 0 || iv->iv.ptr)
	{	/* use last block of previous encrypted message */
		chunk_free(&iv->iv);
		iv->iv = iv->last_block;
		iv->last_block = chunk_empty;
	}
	else
	{
		/* initial phase 2 IV = hash(last_phase1_block | mid) */
		u_int32_t net;;
		chunk_t data;

		net = htonl(iv->mid);
		data = chunk_cata("cc", this->phase1_iv.iv, chunk_from_thing(net));
		if (!this->hasher->allocate_hash(this->hasher, data, &iv->iv))
		{
			return FALSE;
		}
		if (iv->iv.len > this->aead->get_block_size(this->aead))
		{
			iv->iv.len = this->aead->get_block_size(this->aead);
		}
	}
	DBG4(DBG_IKE, "next IV for MID %u %B", iv->mid, &iv->iv);
	return TRUE;
}

/**
 * Try to find an IV for the given message ID, if not found, generate it.
 */
static iv_data_t *lookup_iv(private_keymat_v1_t *this, u_int32_t mid)
{
	enumerator_t *enumerator;
	iv_data_t *iv, *found = NULL;

	if (mid == 0)
	{
		return &this->phase1_iv;
	}

	enumerator = this->ivs->create_enumerator(this->ivs);
	while (enumerator->enumerate(enumerator, &iv))
	{
		if (iv->mid == mid)
		{	/* IV gets moved to the front of the list */
			this->ivs->remove_at(this->ivs, enumerator);
			found = iv;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!found)
	{
		INIT(found,
			.mid = mid,
		);
		if (!generate_iv(this, found))
		{
			iv_data_destroy(found);
			return NULL;
		}
	}
	this->ivs->insert_first(this->ivs, found);
	/* remove least recently used IV if maximum reached */
	if (this->ivs->get_count(this->ivs) > MAX_IV &&
		this->ivs->remove_last(this->ivs, (void**)&iv) == SUCCESS)
	{
		iv_data_destroy(iv);
	}
	return found;
}

METHOD(keymat_v1_t, get_iv, bool,
	private_keymat_v1_t *this, u_int32_t mid, chunk_t *out)
{
	iv_data_t *iv;

	iv = lookup_iv(this, mid);
	if (iv)
	{
		*out = iv->iv;
		return TRUE;
	}
	return FALSE;
}

METHOD(keymat_v1_t, update_iv, bool,
	private_keymat_v1_t *this, u_int32_t mid, chunk_t last_block)
{
	iv_data_t *iv = lookup_iv(this, mid);
	if (iv)
	{	/* update last block */
		chunk_free(&iv->last_block);
		iv->last_block = chunk_clone(last_block);
		return TRUE;
	}
	return FALSE;
}

METHOD(keymat_v1_t, confirm_iv, bool,
	private_keymat_v1_t *this, u_int32_t mid)
{
	iv_data_t *iv = lookup_iv(this, mid);
	if (iv)
	{
		return generate_iv(this, iv);
	}
	return FALSE;
}

METHOD(keymat_t, get_version, ike_version_t,
	private_keymat_v1_t *this)
{
	return IKEV1;
}

METHOD(keymat_t, create_dh, diffie_hellman_t*,
	private_keymat_v1_t *this, diffie_hellman_group_t group)
{
	return lib->crypto->create_dh(lib->crypto, group);
}

METHOD(keymat_t, create_nonce_gen, nonce_gen_t*,
	private_keymat_v1_t *this)
{
	return lib->crypto->create_nonce_gen(lib->crypto);
}

METHOD(keymat_t, get_aead, aead_t*,
	private_keymat_v1_t *this, bool in)
{
	return this->aead;
}

METHOD(keymat_t, destroy, void,
	private_keymat_v1_t *this)
{
	DESTROY_IF(this->prf);
	DESTROY_IF(this->prf_auth);
	DESTROY_IF(this->aead);
	DESTROY_IF(this->hasher);
	chunk_clear(&this->skeyid_d);
	chunk_clear(&this->skeyid_a);
	chunk_free(&this->phase1_iv.iv);
	chunk_free(&this->phase1_iv.last_block);
	this->ivs->destroy_function(this->ivs, (void*)iv_data_destroy);
	this->qms->destroy_function(this->qms, (void*)qm_data_destroy);
	free(this);
}

/**
 * See header
 */
keymat_v1_t *keymat_v1_create(bool initiator)
{
	private_keymat_v1_t *this;

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
			.create_hasher = _create_hasher,
			.get_hasher = _get_hasher,
			.get_hash = _get_hash,
			.get_hash_phase2 = _get_hash_phase2,
			.get_iv = _get_iv,
			.update_iv = _update_iv,
			.confirm_iv = _confirm_iv,
		},
		.ivs = linked_list_create(),
		.qms = linked_list_create(),
		.initiator = initiator,
	);

	return &this->public;
}
