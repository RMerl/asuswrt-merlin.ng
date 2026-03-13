/*
 * Copyright (C) 2015-2020 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
#include <crypto/hashers/hash_algorithm_set.h>

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
	 * Key to build outgoing authentication data (SKp)
	 */
	chunk_t skp_build;

	/**
	 * Key to verify incoming authentication data (SKp)
	 */
	chunk_t skp_verify;

	/**
	 * Set of hash algorithms supported by peer for signature authentication
	 */
	hash_algorithm_set_t *hash_algorithms;
};

METHOD(keymat_t, get_version, ike_version_t,
	private_keymat_v2_t *this)
{
	return IKEV2;
}

METHOD(keymat_t, create_ke, key_exchange_t*,
	private_keymat_v2_t *this, key_exchange_method_t method)
{
	return lib->crypto->create_ke(lib->crypto, method);
}

METHOD(keymat_t, create_nonce_gen, nonce_gen_t*,
	private_keymat_v2_t *this)
{
	return lib->crypto->create_nonce_gen(lib->crypto);
}

/**
 * Create aead_t objects for a combined-mode AEAD algorithm, sets the length of
 * sk_ei and sk_er
 */
static bool create_ike_aead(private_keymat_v2_t *this, uint16_t alg,
							uint16_t key_size, chunk_t *sk_ei, chunk_t *sk_er)
{
	u_int salt_size;

	switch (alg)
	{
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
			/* RFC 4106 */
		case ENCR_CHACHA20_POLY1305:
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

	this->aead_in = lib->crypto->create_aead(lib->crypto, alg, key_size / 8,
											 salt_size);
	this->aead_out = lib->crypto->create_aead(lib->crypto, alg, key_size / 8,
											  salt_size);
	if (!this->aead_in || !this->aead_out)
	{
		DBG1(DBG_IKE, "%N %N (key size %d) not supported!",
			 transform_type_names, ENCRYPTION_ALGORITHM,
			 encryption_algorithm_names, alg, key_size);
		return FALSE;
	}
	sk_ei->len = this->aead_in->get_key_size(this->aead_in);
	sk_er->len = this->aead_out->get_key_size(this->aead_out);
	if (sk_ei->len != sk_er->len)
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * Create aead_t objects for traditional encryption and MAC algorithms, sets the
 * length of key chunks
 */
static bool create_ike_traditional(private_keymat_v2_t *this, uint16_t enc_alg,
					uint16_t enc_size, uint16_t int_alg, chunk_t *sk_ai,
					chunk_t *sk_ar, chunk_t *sk_ei, chunk_t *sk_er)
{
	crypter_t *crypter_i = NULL, *crypter_o = NULL;
	signer_t *signer_i, *signer_o;
	iv_gen_t *ivg_i, *ivg_o;

	signer_i = lib->crypto->create_signer(lib->crypto, int_alg);
	signer_o = lib->crypto->create_signer(lib->crypto, int_alg);
	if (!signer_i || !signer_o)
	{
		DBG1(DBG_IKE, "%N %N not supported!",
			 transform_type_names, INTEGRITY_ALGORITHM,
			 integrity_algorithm_names, int_alg);
		goto failure;
	}
	crypter_i = lib->crypto->create_crypter(lib->crypto, enc_alg, enc_size / 8);
	crypter_o = lib->crypto->create_crypter(lib->crypto, enc_alg, enc_size / 8);
	if (!crypter_i || !crypter_o)
	{
		DBG1(DBG_IKE, "%N %N (key size %d) not supported!",
			 transform_type_names, ENCRYPTION_ALGORITHM,
			 encryption_algorithm_names, enc_alg, enc_size);
		goto failure;
	}
	sk_ai->len = signer_i->get_key_size(signer_i);
	sk_ar->len = signer_o->get_key_size(signer_o);
	if (sk_ai->len != sk_ar->len)
	{
		goto failure;
	}
	sk_ei->len = crypter_i->get_key_size(crypter_i);
	sk_er->len = crypter_o->get_key_size(crypter_o);
	if (sk_ei->len != sk_er->len)
	{
		goto failure;
	}
	ivg_i = iv_gen_create_for_alg(enc_alg);
	ivg_o = iv_gen_create_for_alg(enc_alg);
	if (!ivg_i || !ivg_o)
	{
		goto failure;
	}
	this->aead_in = aead_create(crypter_i, signer_i, ivg_i);
	this->aead_out = aead_create(crypter_o, signer_o, ivg_o);
	signer_i = signer_o = NULL;
	crypter_i = crypter_o = NULL;

failure:
	DESTROY_IF(signer_i);
	DESTROY_IF(signer_o);
	DESTROY_IF(crypter_i);
	DESTROY_IF(crypter_o);
	return this->aead_in && this->aead_out;
}

/**
 * Set keys on AEAD objects
 */
static bool set_aead_keys(private_keymat_v2_t *this, uint16_t enc_alg,
						  chunk_t sk_ai, chunk_t sk_ar,
						  chunk_t sk_ei, chunk_t sk_er)
{
	aead_t *aead_i, *aead_r;
	chunk_t sk_i, sk_r;
	bool success;

	aead_i = this->initiator ? this->aead_out : this->aead_in;
	aead_r = this->initiator ? this->aead_in : this->aead_out;

	sk_i = chunk_cat("cc", sk_ai, sk_ei);
	sk_r = chunk_cat("cc", sk_ar, sk_er);

	success = aead_i->set_key(aead_i, sk_i) &&
			  aead_r->set_key(aead_r, sk_r);

	chunk_clear(&sk_i);
	chunk_clear(&sk_r);
	return success;
}

METHOD(keymat_v2_t, derive_ike_keys, bool,
	private_keymat_v2_t *this, proposal_t *proposal, array_t *kes,
	chunk_t nonce_i, chunk_t nonce_r, ike_sa_id_t *id,
	pseudo_random_function_t rekey_function, chunk_t rekey_skd)
{
	chunk_t skeyseed = chunk_empty, secret, add_secret = chunk_empty;
	chunk_t full_nonce, fixed_nonce, prf_plus_seed, spi_i, spi_r;
	chunk_t keymat = chunk_empty, sk_ei = chunk_empty, sk_er = chunk_empty;
	chunk_t sk_ai = chunk_empty, sk_ar = chunk_empty, sk_pi, sk_pr;
	kdf_t *prf = NULL, *prf_plus = NULL;
	uint16_t prf_alg, key_size, enc_alg, enc_size, int_alg;
	bool success = FALSE;

	spi_i = chunk_alloca(sizeof(uint64_t));
	spi_r = chunk_alloca(sizeof(uint64_t));

	/* create SA's general purpose PRF first, we may use it here */
	if (!proposal->get_algorithm(proposal, PSEUDO_RANDOM_FUNCTION, &prf_alg,
								 NULL))
	{
		DBG1(DBG_IKE, "no %N selected",
			 transform_type_names, PSEUDO_RANDOM_FUNCTION);
		return FALSE;
	}
	this->prf_alg = prf_alg;
	DESTROY_IF(this->prf);
	this->prf = lib->crypto->create_prf(lib->crypto, this->prf_alg);
	if (!this->prf)
	{
		DBG1(DBG_IKE, "%N %N not supported!", transform_type_names,
			 PSEUDO_RANDOM_FUNCTION, pseudo_random_function_names,
			 this->prf_alg);
		return FALSE;
	}
	key_size = this->prf->get_key_size(this->prf);

	/* create SA's AEAD instances to determine key sizes */
	if (!proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM, &enc_alg,
								 &enc_size))
	{
		DBG1(DBG_IKE, "no %N selected", transform_type_names,
			 ENCRYPTION_ALGORITHM);
		return FALSE;
	}
	DESTROY_IF(this->aead_in);
	DESTROY_IF(this->aead_out);
	if (!encryption_algorithm_is_aead(enc_alg))
	{
		if (!proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM, &int_alg,
									 NULL))
		{
			DBG1(DBG_IKE, "no %N selected", transform_type_names,
				 INTEGRITY_ALGORITHM);
			return FALSE;
		}
		if (!create_ike_traditional(this, enc_alg, enc_size, int_alg,
									&sk_ai, &sk_ar, &sk_ei, &sk_er))
		{
			return FALSE;
		}
	}
	else if (!create_ike_aead(this, enc_alg, enc_size, &sk_ei, &sk_er))
	{
		return FALSE;
	}

	if (!key_exchange_concat_secrets(kes, &secret, &add_secret))
	{
		return FALSE;
	}
	DBG4(DBG_IKE, "key exchange secret %B", &secret);
	DBG4(DBG_IKE, "additional key exchange secret %B", &add_secret);
	/* full nonce is used as seed for PRF+ ... */
	full_nonce = chunk_cat("cc", nonce_i, nonce_r);
	DBG4(DBG_IKE, "nonces %B", &full_nonce);
	/* but the PRF may need a fixed key which only uses the first bytes of
	 * the nonces. */
	switch (prf_alg)
	{
		case PRF_AES128_CMAC:
			/* while variable keys may be used according to RFC 4615, RFC 7296
			 * explicitly limits the key size to 128 bit for this application */
		case PRF_AES128_XCBC:
			/* while RFC 4434 defines variable keys for AES-XCBC, RFC 3664 does
			 * not and therefore fixed key semantics apply to XCBC for key
			 * derivation, which is also reinforced by RFC 7296 */
		case PRF_CAMELLIA128_XCBC:
			/* draft-kanno-ipsecme-camellia-xcbc refers to rfc 4434, we
			 * assume fixed key length. */
			nonce_i.len = min(nonce_i.len, key_size / 2);
			nonce_r.len = min(nonce_r.len, key_size / 2);
			break;
		default:
			/* all other algorithms use variable key length, full nonce */
			break;
	}
	fixed_nonce = chunk_cat("cc", nonce_i, nonce_r);

	if (rekey_function == PRF_UNDEFINED)
	{
		/* SKEYSEED = prf(Ni | Nr, g^ir) */
		prf = lib->crypto->create_kdf(lib->crypto, KDF_PRF, this->prf_alg);
		if (!prf)
		{
			DBG1(DBG_IKE, "%N with %N not supported",
				 key_derivation_function_names, KDF_PRF,
				 pseudo_random_function_names, this->prf_alg);
			chunk_clear(&secret);
			chunk_clear(&add_secret);
			chunk_free(&full_nonce);
			chunk_free(&fixed_nonce);
			return FALSE;
		}
		if (prf->set_param(prf, KDF_PARAM_KEY, secret) &&
			prf->set_param(prf, KDF_PARAM_SALT, fixed_nonce) &&
			prf->allocate_bytes(prf, 0, &skeyseed))
		{
			prf_plus = lib->crypto->create_kdf(lib->crypto, KDF_PRF_PLUS,
											   this->prf_alg);
		}
	}
	else
	{
		/* SKEYSEED = prf(SK_d (old), [g^ir (new)] | Ni | Nr)
		 * use OLD SAs PRF functions for both prf_plus and prf */
		prf = lib->crypto->create_kdf(lib->crypto, KDF_PRF, rekey_function);
		if (!prf)
		{
			DBG1(DBG_IKE, "%N with PRF of old SA %N not supported",
				 key_derivation_function_names, KDF_PRF,
				 pseudo_random_function_names, rekey_function);
			chunk_clear(&secret);
			chunk_clear(&add_secret);
			chunk_free(&full_nonce);
			chunk_free(&fixed_nonce);
			return FALSE;
		}
		secret = chunk_cat("scc", secret, full_nonce, add_secret);
		if (prf->set_param(prf, KDF_PARAM_KEY, secret) &&
			prf->set_param(prf, KDF_PARAM_SALT, rekey_skd) &&
			prf->allocate_bytes(prf, 0, &skeyseed))
		{
			prf_plus = lib->crypto->create_kdf(lib->crypto, KDF_PRF_PLUS,
											   rekey_function);
		}
	}
	DBG4(DBG_IKE, "SKEYSEED %B", &skeyseed);
	chunk_clear(&secret);
	chunk_clear(&add_secret);
	chunk_free(&fixed_nonce);
	DESTROY_IF(prf);

	/* KEYMAT = prf+ (SKEYSEED, Ni | Nr | SPIi | SPIr)
	 */
	*((uint64_t*)spi_i.ptr) = id->get_initiator_spi(id);
	*((uint64_t*)spi_r.ptr) = id->get_responder_spi(id);
	prf_plus_seed = chunk_cat("ccc", full_nonce, spi_i, spi_r);

	if (prf_plus &&
		(!prf_plus->set_param(prf_plus, KDF_PARAM_KEY, skeyseed) ||
		 !prf_plus->set_param(prf_plus, KDF_PARAM_SALT, prf_plus_seed)))
	{
		prf_plus->destroy(prf_plus);
		prf_plus = NULL;
	}
	chunk_clear(&skeyseed);
	chunk_free(&full_nonce);
	chunk_free(&prf_plus_seed);

	if (!prf_plus)
	{
		goto failure;
	}

	/* KEYMAT = SK_d | SK_ai | SK_ar | SK_ei | SK_er | SK_pi | SK_pr
	 *
	 * SK_d, SK_pi and SK_pr have the size of the PRF key
	 */
	keymat.len = 3 * key_size + sk_ai.len + sk_ar.len + sk_ei.len + sk_er.len;
	if (!prf_plus->allocate_bytes(prf_plus, keymat.len, &keymat))
	{
		goto failure;
	}
	chunk_clear(&this->skd);
	chunk_split(keymat, "ammmmaa", key_size, &this->skd, sk_ai.len, &sk_ai,
				sk_ar.len, &sk_ar, sk_ei.len, &sk_ei, sk_er.len, &sk_er,
				key_size, &sk_pi, key_size, &sk_pr);

	/* SK_d is used for generating CHILD_SA key mat => store for later use */
	DBG4(DBG_IKE, "Sk_d secret %B", &this->skd);
	if (!encryption_algorithm_is_aead(enc_alg))
	{	/* SK_ai/SK_ar used for integrity protection */
		DBG4(DBG_IKE, "Sk_ai secret %B", &sk_ai);
		DBG4(DBG_IKE, "Sk_ar secret %B", &sk_ar);
	}
	/* SK_ei/SK_er used for encryption */
	DBG4(DBG_IKE, "Sk_ei secret %B", &sk_ei);
	DBG4(DBG_IKE, "Sk_er secret %B", &sk_er);
	if (!set_aead_keys(this, enc_alg, sk_ai, sk_ar, sk_ei, sk_er))
	{
		goto failure;
	}
	/* SK_pi/SK_pr used for authentication => stored for later */
	DBG4(DBG_IKE, "Sk_pi secret %B", &sk_pi);
	DBG4(DBG_IKE, "Sk_pr secret %B", &sk_pr);
	chunk_clear(&this->skp_build);
	chunk_clear(&this->skp_verify);
	if (this->initiator)
	{
		this->skp_build = sk_pi;
		this->skp_verify = sk_pr;
	}
	else
	{
		this->skp_build = sk_pr;
		this->skp_verify = sk_pi;
	}
	charon->bus->ike_derived_keys(charon->bus, this->skd, sk_ai, sk_ar,
								  sk_ei, sk_er, sk_pi, sk_pr);
	success = TRUE;

failure:
	chunk_clear(&keymat);
	DESTROY_IF(prf_plus);
	return success;
}

/**
 * Derives a new key from the given PPK and old key
 */
static bool derive_ppk_key(private_keymat_v2_t *this, char *name, chunk_t ppk,
						   chunk_t key, chunk_t *new_key)
{
	kdf_t *prf_plus;

	prf_plus = lib->crypto->create_kdf(lib->crypto, KDF_PRF_PLUS, this->prf_alg);
	if (!prf_plus ||
		!prf_plus->set_param(prf_plus, KDF_PARAM_KEY, ppk) ||
		!prf_plus->set_param(prf_plus, KDF_PARAM_SALT, key) ||
		!prf_plus->allocate_bytes(prf_plus, key.len, new_key))
	{
		DBG1(DBG_IKE, "unable to derive %s with PPK", name);
		DESTROY_IF(prf_plus);
		return FALSE;
	}
	prf_plus->destroy(prf_plus);
	return TRUE;
}

METHOD(keymat_v2_t, derive_ike_keys_ppk, bool,
	private_keymat_v2_t *this, chunk_t ppk)
{
	chunk_t skd = chunk_empty, new_skpi = chunk_empty, new_skpr = chunk_empty;
	chunk_t *skpi, *skpr;

	if (!this->skd.ptr)
	{
		return FALSE;
	}

	if (this->initiator)
	{
		skpi = &this->skp_build;
		skpr = &this->skp_verify;
	}
	else
	{
		skpi = &this->skp_verify;
		skpr = &this->skp_build;
	}

	DBG4(DBG_IKE, "derive keys using PPK %B", &ppk);

	if (!derive_ppk_key(this, "Sk_d", ppk, this->skd, &skd) ||
		!derive_ppk_key(this, "Sk_pi", ppk, *skpi, &new_skpi) ||
		!derive_ppk_key(this, "Sk_pr", ppk, *skpr, &new_skpr))
	{
		chunk_clear(&skd);
		chunk_clear(&new_skpi);
		chunk_clear(&new_skpr);
		return FALSE;
	}

	DBG4(DBG_IKE, "Sk_d secret %B", &skd);
	chunk_clear(&this->skd);
	this->skd = skd;

	DBG4(DBG_IKE, "Sk_pi secret %B", &new_skpi);
	chunk_clear(skpi);
	*skpi = new_skpi;

	DBG4(DBG_IKE, "Sk_pr secret %B", &new_skpr);
	chunk_clear(skpr);
	*skpr = new_skpr;
	return TRUE;
}

METHOD(keymat_v2_t, derive_child_keys, bool,
	private_keymat_v2_t *this, proposal_t *proposal, array_t *kes,
	chunk_t nonce_i, chunk_t nonce_r, chunk_t *encr_i, chunk_t *integ_i,
	chunk_t *encr_r, chunk_t *integ_r)
{
	uint16_t enc_alg, int_alg, enc_size = 0, int_size = 0;
	chunk_t seed, secret = chunk_empty, add_secret = chunk_empty;
	chunk_t keymat = chunk_empty;
	kdf_t *prf_plus;

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
			case ENCR_CAMELLIA_CTR:
			case ENCR_NULL_AUTH_AES_GMAC:
			case ENCR_CHACHA20_POLY1305:
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

	if (kes)
	{
		if (!key_exchange_concat_secrets(kes, &secret, &add_secret))
		{
			return FALSE;
		}
		DBG4(DBG_CHD, "key exchange secret %B", &secret);
		DBG4(DBG_CHD, "additional key exchange secret %B", &add_secret);
	}
	seed = chunk_cata("sccs", secret, nonce_i, nonce_r, add_secret);
	DBG4(DBG_CHD, "seed %B", &seed);

	prf_plus = lib->crypto->create_kdf(lib->crypto, KDF_PRF_PLUS, this->prf_alg);
	if (!prf_plus ||
		!prf_plus->set_param(prf_plus, KDF_PARAM_KEY, this->skd) ||
		!prf_plus->set_param(prf_plus, KDF_PARAM_SALT, seed))
	{
		DESTROY_IF(prf_plus);
		memwipe(seed.ptr, seed.len);
		return FALSE;
	}
	memwipe(seed.ptr, seed.len);

	*encr_i = *integ_i = *encr_r = *integ_r = chunk_empty;
	keymat.len = 2 * enc_size + 2 * int_size;
	if (!prf_plus->allocate_bytes(prf_plus, keymat.len, &keymat))
	{
		prf_plus->destroy(prf_plus);
		return FALSE;
	}
	prf_plus->destroy(prf_plus);

	chunk_split(keymat, "aaaa", enc_size, encr_i, int_size, integ_i,
				enc_size, encr_r, int_size, integ_r);
	chunk_clear(&keymat);

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

METHOD(keymat_v2_t, get_int_auth, bool,
	private_keymat_v2_t *this, bool verify, chunk_t data, chunk_t prev,
	chunk_t *auth)
{
	chunk_t skp;

	skp = verify ? this->skp_verify : this->skp_build;

	DBG3(DBG_IKE, "IntAuth_N-1 %B", &prev);
	DBG3(DBG_IKE, "IntAuth_A|P %B", &data);
	DBG4(DBG_IKE, "SK_p %B", &skp);
	if (!this->prf->set_key(this->prf, skp) ||
		!this->prf->allocate_bytes(this->prf, prev, NULL) ||
		!this->prf->allocate_bytes(this->prf, data, auth))
	{
		return FALSE;
	}
	DBG3(DBG_IKE, "IntAuth_N = prf(Sk_px, data) %B", auth);
	return TRUE;
}

METHOD(keymat_v2_t, get_auth_octets, bool,
	private_keymat_v2_t *this, bool verify, chunk_t ike_sa_init,
	chunk_t nonce, chunk_t int_auth, chunk_t ppk, identification_t *id,
	char reserved[3], chunk_t *octets, array_t *schemes)
{
	chunk_t chunk, idx;
	chunk_t skp_ppk = chunk_empty;
	chunk_t skp;

	skp = verify ? this->skp_verify : this->skp_build;
	if (ppk.ptr)
	{
		DBG4(DBG_IKE, "PPK %B", &ppk);
		if (!derive_ppk_key(this, "SK_p", ppk, skp, &skp_ppk))
		{
			return FALSE;
		}
		skp = skp_ppk;
	}

	chunk = chunk_alloca(4);
	chunk.ptr[0] = id->get_type(id);
	memcpy(chunk.ptr + 1, reserved, 3);
	idx = chunk_cata("cc", chunk, id->get_encoding(id));

	DBG3(DBG_IKE, "IDx' %B", &idx);
	DBG4(DBG_IKE, "SK_p %B", &skp);
	if (!this->prf->set_key(this->prf, skp) ||
		!this->prf->allocate_bytes(this->prf, idx, &chunk))
	{
		chunk_clear(&skp_ppk);
		return FALSE;
	}
	chunk_clear(&skp_ppk);
	*octets = chunk_cat("ccmc", ike_sa_init, nonce, chunk, int_auth);
	DBG3(DBG_IKE, "octets = message + nonce + prf(Sk_px, IDx') + IntAuth %B",
		 octets);
	return TRUE;
}

/**
 * Key pad for the AUTH method SHARED_KEY_MESSAGE_INTEGRITY_CODE.
 */
#define IKEV2_KEY_PAD "Key Pad for IKEv2"
#define IKEV2_KEY_PAD_LENGTH 17

METHOD(keymat_v2_t, get_psk_sig, bool,
	private_keymat_v2_t *this, bool verify, chunk_t ike_sa_init,
	chunk_t nonce, chunk_t int_auth, chunk_t secret, chunk_t ppk,
	identification_t *id, char reserved[3], chunk_t *sig)
{
	chunk_t skp_ppk = chunk_empty, key = chunk_empty, octets = chunk_empty;
	chunk_t key_pad;
	bool success = FALSE;

	if (!secret.len)
	{	/* EAP uses SK_p if no MSK has been established */
		secret = verify ? this->skp_verify : this->skp_build;
		if (ppk.ptr)
		{
			if (!derive_ppk_key(this, "SK_p", ppk, secret, &skp_ppk))
			{
				return FALSE;
			}
			secret = skp_ppk;
		}
	}
	if (!get_auth_octets(this, verify, ike_sa_init, nonce, int_auth, ppk, id,
						 reserved, &octets, NULL))
	{
		goto failure;
	}
	/* AUTH = prf(prf(Shared Secret,"Key Pad for IKEv2"), <msg octets>) */
	key_pad = chunk_create(IKEV2_KEY_PAD, IKEV2_KEY_PAD_LENGTH);
	if (!this->prf->set_key(this->prf, secret) ||
		!this->prf->allocate_bytes(this->prf, key_pad, &key))
	{
		goto failure;
	}
	if (!this->prf->set_key(this->prf, key) ||
		!this->prf->allocate_bytes(this->prf, octets, sig))
	{
		goto failure;
	}
	DBG4(DBG_IKE, "secret %B", &secret);
	DBG4(DBG_IKE, "prf(secret, keypad) %B", &key);
	DBG3(DBG_IKE, "AUTH = prf(prf(secret, keypad), octets) %B", sig);
	success = TRUE;

failure:
	chunk_clear(&skp_ppk);
	chunk_free(&octets);
	chunk_free(&key);
	return success;
}

METHOD(keymat_v2_t, hash_algorithm_supported, bool,
	private_keymat_v2_t *this, hash_algorithm_t hash)
{
	if (!this->hash_algorithms)
	{
		return FALSE;
	}
	return this->hash_algorithms->contains(this->hash_algorithms, hash);
}

METHOD(keymat_v2_t, add_hash_algorithm, void,
	private_keymat_v2_t *this, hash_algorithm_t hash)
{
	if (!this->hash_algorithms)
	{
		this->hash_algorithms = hash_algorithm_set_create();
	}
	this->hash_algorithms->add(this->hash_algorithms, hash);
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
	DESTROY_IF(this->hash_algorithms);
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
				.create_ke = _create_ke,
				.create_nonce_gen = _create_nonce_gen,
				.get_aead = _get_aead,
				.destroy = _destroy,
			},
			.derive_ike_keys = _derive_ike_keys,
			.derive_ike_keys_ppk = _derive_ike_keys_ppk,
			.derive_child_keys = _derive_child_keys,
			.get_skd = _get_skd,
			.get_int_auth = _get_int_auth,
			.get_auth_octets = _get_auth_octets,
			.get_psk_sig = _get_psk_sig,
			.add_hash_algorithm = _add_hash_algorithm,
			.hash_algorithm_supported = _hash_algorithm_supported,

		},
		.initiator = initiator,
		.prf_alg = PRF_UNDEFINED,
	);

	return &this->public;
}
