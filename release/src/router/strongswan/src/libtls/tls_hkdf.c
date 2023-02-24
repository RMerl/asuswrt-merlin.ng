/*
 * Copyright (C) 2020 Tobias Brunner
 * Copyright (C) 2020 Pascal Knecht
 * Copyright (C) 2020 Méline Sieber
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

#include "tls_hkdf.h"

#include <bio/bio_writer.h>

typedef struct private_tls_hkdf_t private_tls_hkdf_t;

typedef struct cached_secrets_t {
	chunk_t client;
	chunk_t server;
} cached_secrets_t;

typedef enum hkdf_phase {
	HKDF_PHASE_0,
	HKDF_PHASE_1,
	HKDF_PHASE_2,
	HKDF_PHASE_3,
} hkdf_phase;

struct private_tls_hkdf_t {

	/**
	 * Public tls_hkdf_t interface.
	 */
	struct tls_hkdf_t public;

	/**
	 * Phase we are in.
	 */
	hkdf_phase phase;

	/**
	 * Pseudorandom function used.
	 */
	prf_t *prf;

	/**
	 * prf+ implementation.
	 */
	kdf_t *prf_plus;

	/**
	 * Hasher used.
	 */
	hasher_t *hasher;

	/**
	 * (EC)DHE as IKM to switch from phase 1 to phase 2
	 */
	chunk_t shared_secret;

	/**
	 * PSK used.
	 */
	chunk_t psk;

	/**
	 * PRK used.
	 */
	chunk_t prk;

	/**
	 * Handshake traffic secrets.
	 */
	cached_secrets_t handshake_traffic_secrets;

	/**
	 * Current traffic secrets.
	 */
	cached_secrets_t traffic_secrets;
};

static char *hkdf_labels[] = {
	"ext binder",
	"res binder",
	"c e traffic",
	"e exp master",
	"c hs traffic",
	"s hs traffic",
	"c ap traffic",
	"s ap traffic",
	"exp master",
	"res master",
};

/**
 * Step 1: Extract, as defined in RFC 5869, section 2.2:
 * HKDF-Extract(salt, IKM) -> PRK
 */
static bool extract(private_tls_hkdf_t *this, chunk_t salt, chunk_t ikm,
					chunk_t *prk)
{
	if (!this->prf->set_key(this->prf, salt))
	{
		DBG1(DBG_TLS, "unable to set PRF secret to salt");
		return FALSE;
	}
	chunk_clear(prk);
	if(!this->prf->allocate_bytes(this->prf, ikm, prk))
	{
		DBG1(DBG_TLS, "unable to allocate PRF result");
		return FALSE;
	}

	DBG4(DBG_TLS, "PRK: %B", prk);
	return TRUE;
}

/**
 * Step 2: Expand as defined in RFC 5869, section 2.3:
 * HKDF-Expand(PRK, info, L) -> OKM
 */
static bool expand(private_tls_hkdf_t *this, chunk_t prk, chunk_t info,
				   size_t length, chunk_t *okm)
{
	if (!this->prf_plus->set_param(this->prf_plus, KDF_PARAM_KEY, prk) ||
		!this->prf_plus->set_param(this->prf_plus, KDF_PARAM_SALT, info) ||
		!this->prf_plus->allocate_bytes(this->prf_plus, length, okm))
	{
		DBG1(DBG_TLS, "unable to allocate PRF+ result");
		return FALSE;
	}

	DBG4(DBG_TLS, "OKM: %B", okm);
	return TRUE;
}

/**
 * Expand-Label as defined in RFC 8446, section 7.1:
 * HKDF-Expand-Label(Secret, Label, Context, Length) -> OKM
 */
static bool expand_label(private_tls_hkdf_t *this, chunk_t secret,
						 chunk_t label, chunk_t context, uint16_t length,
						 chunk_t *key)
{
	bool success;

	if (!label.len || label.len > 249 || context.len > 255)
	{
		return FALSE;
	}

	/* HKDFLabel as defined in RFC 8446, section 7.1 */
	bio_writer_t *writer = bio_writer_create(0);
	writer->write_uint16(writer, length);
	label = chunk_cata("cc", chunk_from_str("tls13 "), label);
	writer->write_data8(writer, label);
	writer->write_data8(writer, context);

	success = expand(this, secret, writer->get_buf(writer), length, key);
	writer->destroy(writer);
	return success;
}

/**
 * Derive-Secret as defined in RFC 8446, section 7.1:
 * Derive-Secret(Secret, Label, Message) -> OKM
 */
static bool derive_secret(private_tls_hkdf_t *this, chunk_t secret,
						  chunk_t label, chunk_t messages, chunk_t *okm)
{
	chunk_t context;
	bool success;

	if (!this->hasher->allocate_hash(this->hasher, messages, &context))
	{
		return FALSE;
	}

	success = expand_label(this, secret, label, context,
						   this->hasher->get_hash_size(this->hasher), okm);
	chunk_free(&context);
	return success;
}

/**
 * Move to phase 1 (Early Secret)
 *
 *            0
 *            |
 *            v
 *  PSK ->  HKDF-Extract = Early Secret
 *            |
 *            +-----> Derive-Secret(., "ext binder" | "res binder", "")
 *            |                     = binder_key
 *            |
 *            +-----> Derive-Secret(., "c e traffic", ClientHello)
 *            |                     = client_early_traffic_secret
 *            |
 *            +-----> Derive-Secret(., "e exp master", ClientHello)
 *            |                     = early_exporter_master_secret
 *            v
 */
static bool move_to_phase_1(private_tls_hkdf_t *this)
{
	chunk_t salt_zero, psk = this->psk;

	switch (this->phase)
	{
		case HKDF_PHASE_0:
			salt_zero = chunk_alloca(this->hasher->get_hash_size(this->hasher));
			chunk_copy_pad(salt_zero, chunk_empty, 0);
			if (!psk.ptr)
			{
				psk = salt_zero;
			}
			if (!extract(this, salt_zero, psk, &this->prk))
			{
				DBG1(DBG_TLS, "unable to extract PRK");
				return FALSE;
			}
			this->phase = HKDF_PHASE_1;
			return TRUE;
		case HKDF_PHASE_1:
			return TRUE;
		default:
			DBG1(DBG_TLS, "invalid HKDF phase");
			return FALSE;
	}
}

/**
 * Move to phase 2 (Handshake Secret)
 *
 *      Derive-Secret(., "derived", "")
 *            |
 *            v
 *  (EC)DHE -> HKDF-Extract = Handshake Secret
 *            |
 *            +-----> Derive-Secret(., "c hs traffic",
 *            |                     ClientHello...ServerHello)
 *            |                     = client_handshake_traffic_secret
 *            |
 *            +-----> Derive-Secret(., "s hs traffic",
 *            |                     ClientHello...ServerHello)
 *            |                     = server_handshake_traffic_secret
 *            v
 */
static bool move_to_phase_2(private_tls_hkdf_t *this)
{
	chunk_t okm;

	switch (this->phase)
	{
		case HKDF_PHASE_0:
			if (!move_to_phase_1(this))
			{
				DBG1(DBG_TLS, "unable to move to phase 1");
				return FALSE;
			}
			/* fall-through */
		case HKDF_PHASE_1:
			if (!derive_secret(this, this->prk, chunk_from_str("derived"),
							   chunk_empty, &okm))
			{
				DBG1(DBG_TLS, "unable to derive secret");
				return FALSE;
			}

			if (!this->shared_secret.ptr)
			{
				DBG1(DBG_TLS, "no shared secret set");
				chunk_clear(&okm);
				return FALSE;
			}

			if (!extract(this, okm, this->shared_secret, &this->prk))
			{
				DBG1(DBG_TLS, "unable extract PRK");
				chunk_clear(&okm);
				return FALSE;
			}
			chunk_clear(&okm);
			this->phase = HKDF_PHASE_2;
			return TRUE;
		case HKDF_PHASE_2:
			return TRUE;
		default:
			DBG1(DBG_TLS, "invalid HKDF phase");
			return FALSE;
	}
}

/**
 * Move to phase 3 (Master Secret)
 *
 *      Derive-Secret(., "derived", "")
 *            |
 *            v
 *  0 -> HKDF-Extract = Master Secret
 *            |
 *            +-----> Derive-Secret(., "c ap traffic",
 *            |                     ClientHello...server Finished)
 *            |                     = client_application_traffic_secret_0
 *            |
 *            +-----> Derive-Secret(., "s ap traffic",
 *            |                     ClientHello...server Finished)
 *            |                     = server_application_traffic_secret_0
 *            |
 *            +-----> Derive-Secret(., "exp master",
 *            |                     ClientHello...server Finished)
 *            |                     = exporter_master_secret
 *            |
 *            +-----> Derive-Secret(., "res master",
 *                                  ClientHello...client Finished)
 *                                  = resumption_master_secret
 */
static bool move_to_phase_3(private_tls_hkdf_t *this)
{
	chunk_t okm, ikm_zero;

	switch (this->phase)
	{
		case HKDF_PHASE_0:
		case HKDF_PHASE_1:
			if (!move_to_phase_2(this))
			{
				DBG1(DBG_TLS, "unable to move to phase 2");
				return FALSE;
			}
			/* fall-through */
		case HKDF_PHASE_2:
			/* prepare okm for next extract */
			if (!derive_secret(this, this->prk, chunk_from_str("derived"),
							   chunk_empty, &okm))
			{
				DBG1(DBG_TLS, "unable to derive secret");
				return FALSE;
			}

			ikm_zero = chunk_alloca(this->hasher->get_hash_size(this->hasher));
			chunk_copy_pad(ikm_zero, chunk_empty, 0);
			if (!extract(this, okm, ikm_zero, &this->prk))
			{
				DBG1(DBG_TLS, "unable extract PRK");
				chunk_clear(&okm);
				return FALSE;
			}
			chunk_clear(&okm);
			this->phase = HKDF_PHASE_3;
			return TRUE;
		case HKDF_PHASE_3:
			return TRUE;
		default:
			DBG1(DBG_TLS, "invalid HKDF phase");
			return FALSE;
	}
}

METHOD(tls_hkdf_t, set_shared_secret, void,
	private_tls_hkdf_t *this, chunk_t shared_secret)
{
	this->shared_secret = chunk_clone(shared_secret);
}

METHOD(tls_hkdf_t, generate_secret, bool,
	private_tls_hkdf_t *this, tls_hkdf_label_t label, chunk_t messages,
	chunk_t *secret)
{
	chunk_t okm;

	switch (label)
	{
		case TLS_HKDF_EXT_BINDER:
		case TLS_HKDF_RES_BINDER:
		case TLS_HKDF_C_E_TRAFFIC:
		case TLS_HKDF_E_EXP_MASTER:
			if (!move_to_phase_1(this))
			{
				DBG1(DBG_TLS, "unable to move to phase 1");
				return FALSE;
			}
			break;
		case TLS_HKDF_C_HS_TRAFFIC:
		case TLS_HKDF_S_HS_TRAFFIC:
			if (!move_to_phase_2(this))
			{
				DBG1(DBG_TLS, "unable to move to phase 2");
				return FALSE;
			}
			break;
		case TLS_HKDF_C_AP_TRAFFIC:
		case TLS_HKDF_S_AP_TRAFFIC:
		case TLS_HKDF_EXP_MASTER:
		case TLS_HKDF_RES_MASTER:
			if (!move_to_phase_3(this))
			{
				DBG1(DBG_TLS, "unable to move to phase 3");
				return FALSE;
			}
			break;
		case TLS_HKDF_UPD_C_TRAFFIC:
		case TLS_HKDF_UPD_S_TRAFFIC:
			if (this->phase != HKDF_PHASE_3)
			{
				DBG1(DBG_TLS, "unable to update traffic keys");
				return FALSE;
			}
			break;
		default:
			DBG1(DBG_TLS, "invalid HKDF label");
			return FALSE;
	}

	if (label == TLS_HKDF_UPD_C_TRAFFIC || label == TLS_HKDF_UPD_S_TRAFFIC)
	{
		chunk_t previous = this->traffic_secrets.client;

		if (label == TLS_HKDF_UPD_S_TRAFFIC)
		{
			previous = this->traffic_secrets.server;
		}

		if (!expand_label(this, previous, chunk_from_str("traffic upd"),
						  chunk_empty, this->hasher->get_hash_size(this->hasher),
						  &okm))
		{
			DBG1(DBG_TLS, "unable to update secret");
			return FALSE;
		}
	}
	else
	{
		if (!derive_secret(this, this->prk, chunk_from_str(hkdf_labels[label]),
						   messages, &okm))
		{
			DBG1(DBG_TLS, "unable to derive secret");
			return FALSE;
		}
	}

	switch (label)
	{
		case TLS_HKDF_C_HS_TRAFFIC:
			chunk_clear(&this->handshake_traffic_secrets.client);
			this->handshake_traffic_secrets.client = chunk_clone(okm);
			/* fall-through */
		case TLS_HKDF_C_AP_TRAFFIC:
		case TLS_HKDF_UPD_C_TRAFFIC:
			chunk_clear(&this->traffic_secrets.client);
			this->traffic_secrets.client = chunk_clone(okm);
			break;
		case TLS_HKDF_S_HS_TRAFFIC:
			chunk_clear(&this->handshake_traffic_secrets.server);
			this->handshake_traffic_secrets.server = chunk_clone(okm);
			/* fall-through */
		case TLS_HKDF_S_AP_TRAFFIC:
		case TLS_HKDF_UPD_S_TRAFFIC:
			chunk_clear(&this->traffic_secrets.server);
			this->traffic_secrets.server = chunk_clone(okm);
			break;
		default:
			break;
	}

	if (secret)
	{
		*secret = okm;
	}
	else
	{
		chunk_clear(&okm);
	}
	return TRUE;
}

/**
 * Derive keys/IVs from the current traffic secrets.
 */
static bool get_shared_label_keys(private_tls_hkdf_t *this, chunk_t label,
								  cached_secrets_t *secrets,
								  bool server, size_t length, chunk_t *key)
{
	chunk_t result = chunk_empty, secret;

	secret = server ? secrets->server : secrets->client;

	if (!expand_label(this, secret, label, chunk_empty, length, &result))
	{
		DBG1(DBG_TLS, "unable to derive labeled secret");
		chunk_clear(&result);
		return FALSE;
	}

	if (key)
	{
		*key = result;
	}
	else
	{
		chunk_clear(&result);
	}
	return TRUE;
}

METHOD(tls_hkdf_t, derive_key, bool,
	private_tls_hkdf_t *this, bool is_server, size_t length, chunk_t *key)
{
	return get_shared_label_keys(this, chunk_from_str("key"),
								 &this->traffic_secrets, is_server, length, key);
}

METHOD(tls_hkdf_t, derive_iv, bool,
	private_tls_hkdf_t *this, bool is_server, size_t length, chunk_t *iv)
{
	return get_shared_label_keys(this, chunk_from_str("iv"),
								 &this->traffic_secrets, is_server, length, iv);
}

METHOD(tls_hkdf_t, derive_finished, bool,
	private_tls_hkdf_t *this, bool server, chunk_t *finished)
{
	return get_shared_label_keys(this, chunk_from_str("finished"),
								 &this->handshake_traffic_secrets, server,
								 this->hasher->get_hash_size(this->hasher),
								 finished);
}

METHOD(tls_hkdf_t, export, bool,
	private_tls_hkdf_t *this, char *label, chunk_t context,
	chunk_t messages, size_t length, chunk_t *key)
{
	chunk_t exporter_master, exporter, hash = chunk_empty;

	if (this->phase != HKDF_PHASE_3)
	{
		DBG1(DBG_TLS, "unable to export key material");
		return FALSE;
	}

	/**
	 * Export key material according to RFC 8446, section 7.5:
	 *
	 * TLS-Exporter(label, context_value, key_length) =
	 *    HKDF-Expand-Label(Derive-Secret(Secret, label, ""),
	 *                      "exporter", Hash(context_value), key_length)
	 */
	if (!generate_secret(this, TLS_HKDF_EXP_MASTER, messages, &exporter_master))
	{
		DBG1(DBG_TLS, "unable to derive exporter master secret");
		return FALSE;
	}

	if (!derive_secret(this, exporter_master, chunk_from_str(label),
					   chunk_empty, &exporter))
	{
		DBG1(DBG_TLS, "unable to derive exporter secret");
		chunk_clear(&exporter_master);
		return FALSE;
	}
	chunk_clear(&exporter_master);

	if (!this->hasher->allocate_hash(this->hasher, context, &hash) ||
		!expand_label(this, exporter, chunk_from_str("exporter"), hash,
					  length, key))
	{
		DBG1(DBG_TLS, "unable to expand key material");
		chunk_clear(&exporter);
		chunk_free(&hash);
		return FALSE;
	}
	chunk_clear(&exporter);
	chunk_free(&hash);
	return TRUE;
}

METHOD(tls_hkdf_t, resume, bool,
	private_tls_hkdf_t *this, chunk_t messages, chunk_t nonce, chunk_t *key)
{
	chunk_t resumption_master;

	if (this->phase != HKDF_PHASE_3)
	{
		DBG1(DBG_TLS, "unable to generate resumption key material");
		return FALSE;
	}
	if (!nonce.len)
	{
		DBG1(DBG_TLS, "no nonce provided");
		return FALSE;
	}

	/**
	 * PSK associated with the ticket according to RFC 8446, section 4.6.1
	 *
	 *    HKDF-Expand-Label(resumption_master_secret,
	 *                      "resumption", ticket_nonce, Hash.length)
	 */
	if (!generate_secret(this, TLS_HKDF_RES_MASTER, messages,
						 &resumption_master))
	{
		DBG1(DBG_TLS, "unable to derive resumption master secret");
		return FALSE;
	}

	if (!expand_label(this, resumption_master, chunk_from_str("resumption"),
					  nonce, this->hasher->get_hash_size(this->hasher), key))
	{
		chunk_clear(&resumption_master);
		DBG1(DBG_TLS, "unable to expand key material");
		return FALSE;
	}
	chunk_clear(&resumption_master);
	return TRUE;
}

METHOD(tls_hkdf_t, binder, bool,
	private_tls_hkdf_t *this, chunk_t seed, chunk_t *out)
{
	chunk_t binder_key, finished_key;

	if (!generate_secret(this, TLS_HKDF_RES_BINDER, chunk_empty, &binder_key))
	{
		DBG1(DBG_TLS, "unable to derive binder key");
		return FALSE;
	}

	if (!expand_label(this, binder_key, chunk_from_str("finished"), chunk_empty,
					  this->hasher->get_hash_size(this->hasher), &finished_key))
	{
		chunk_clear(&binder_key);
		return FALSE;
	}
	chunk_clear(&binder_key);

	if (!this->prf->set_key(this->prf, finished_key) ||
		!this->prf->allocate_bytes(this->prf, seed, out))
	{
		chunk_clear(&finished_key);
		return FALSE;
	}
	chunk_clear(&finished_key);
	return TRUE;
}

METHOD(tls_hkdf_t, allocate_bytes, bool,
	private_tls_hkdf_t *this, chunk_t key, chunk_t seed,
	chunk_t *out)
{
	return this->prf->set_key(this->prf, key) &&
		   this->prf->allocate_bytes(this->prf, seed, out);
}

/**
 * Clean up secrets
 */
static void destroy_secrets(cached_secrets_t *secrets)
{
	chunk_clear(&secrets->client);
	chunk_clear(&secrets->server);
}

METHOD(tls_hkdf_t, destroy, void,
	private_tls_hkdf_t *this)
{
	chunk_clear(&this->psk);
	chunk_clear(&this->prk);
	chunk_clear(&this->shared_secret);
	destroy_secrets(&this->handshake_traffic_secrets);
	destroy_secrets(&this->traffic_secrets);
	DESTROY_IF(this->prf);
	DESTROY_IF(this->prf_plus);
	DESTROY_IF(this->hasher);
	free(this);
}

tls_hkdf_t *tls_hkdf_create(hash_algorithm_t hash_algorithm, chunk_t psk)
{
	private_tls_hkdf_t *this;
	pseudo_random_function_t prf_algorithm;

	switch (hash_algorithm)
	{
		case HASH_SHA256:
			prf_algorithm = PRF_HMAC_SHA2_256;
			break;
		case HASH_SHA384:
			prf_algorithm = PRF_HMAC_SHA2_384;
			break;
		default:
			DBG1(DBG_TLS, "unsupported hash algorithm %N", hash_algorithm_names,
				 hash_algorithm);
			return NULL;
	}

	INIT(this,
		.public = {
			.set_shared_secret = _set_shared_secret,
			.generate_secret = _generate_secret,
			.derive_key = _derive_key,
			.derive_iv = _derive_iv,
			.derive_finished = _derive_finished,
			.export = _export,
			.resume = _resume,
			.binder = _binder,
			.allocate_bytes = _allocate_bytes,
			.destroy = _destroy,
		},
		.phase = HKDF_PHASE_0,
		.psk = psk.ptr ? chunk_clone(psk) : chunk_empty,
		.prf = lib->crypto->create_prf(lib->crypto, prf_algorithm),
		.prf_plus = lib->crypto->create_kdf(lib->crypto, KDF_PRF_PLUS,
											prf_algorithm),
		.hasher = lib->crypto->create_hasher(lib->crypto, hash_algorithm),
	);

	if (!this->prf || !this->prf_plus || !this->hasher)
	{
		if (!this->prf)
		{
			DBG1(DBG_TLS, "%N not supported", pseudo_random_function_names,
				 prf_algorithm);
		}
		if (!this->prf_plus)
		{
			DBG1(DBG_TLS, "%N (%N) not supported", key_derivation_function_names,
				 KDF_PRF_PLUS, pseudo_random_function_names, prf_algorithm);
		}
		if (!this->hasher)
		{
			DBG1(DBG_TLS, "%N not supported", hash_algorithm_names,
				 hash_algorithm);
		}
		DBG1(DBG_TLS, "unable to initialize HKDF");
		destroy(this);
		return NULL;
	}
	return &this->public;
}
