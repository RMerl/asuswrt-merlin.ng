/*
 * Copyright (C) 2016-2019 Andreas Steffen
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

#include "drbg_hmac.h"

#define MAX_DRBG_REQUESTS	0xfffffffe	/* 2^32 - 2 */
#define MAX_DRBG_BYTES		0x00010000	/* 2^19 bits = 2^16 bytes */

typedef struct private_drbg_hmac_t private_drbg_hmac_t;

/**
 * Private data of an drbg_prf_t object.
 */
struct private_drbg_hmac_t {

	/**
	 * Public drbg_prf_t interface.
	 */
	drbg_hmac_t public;

	/**
	 * DRBG type.
	 */
	drbg_type_t type;

	/**
	 * Security strength in bits.
	 */
	uint32_t strength;

	/**
	 * Number of requests for pseudorandom bits
	 */
	size_t reseed_counter;

	/**
	 * Maximum number of requests for pseudorandom bits
	 */
	size_t max_requests;

	/**
	 * True entropy source
	 */
	rng_t *entropy;

	/**
	 * HMAC PRF used by the DRBG
	 */
	prf_t *prf;

	/**
	 * Internal state of HMAC: key
	 */
	chunk_t key;

	/**
	 * Internal state of HMAC: value
	 */
	chunk_t value;

	/**
	 * reference count
	 */
	refcount_t ref;

};

METHOD(drbg_t, get_type, drbg_type_t,
	private_drbg_hmac_t *this)
{
	return this->type;
}

METHOD(drbg_t, get_strength, uint32_t,
	private_drbg_hmac_t *this)
{
	return this->strength;
}

/**
 * Update the internal state of the HMAC_DRBG
 */
static bool update(private_drbg_hmac_t *this, chunk_t data)
{
	chunk_t ch_00 = chunk_from_chars(0x00);
	chunk_t ch_01 = chunk_from_chars(0x01);

	if (!this->prf->set_key(this->prf, this->key) ||
		!this->prf->get_bytes(this->prf, this->value, NULL) ||
	    !this->prf->get_bytes(this->prf, ch_00, NULL) ||
	    !this->prf->get_bytes(this->prf, data, this->key.ptr) ||
		!this->prf->set_key(this->prf, this->key) ||
	    !this->prf->get_bytes(this->prf, this->value, this->value.ptr))
	{
		return FALSE;
	}

	if (data.len > 0)
	{
		if (!this->prf->set_key(this->prf, this->key) ||
			!this->prf->get_bytes(this->prf, this->value, NULL) ||
			!this->prf->get_bytes(this->prf, ch_01, NULL) ||
			!this->prf->get_bytes(this->prf, data, this->key.ptr) ||
			!this->prf->set_key(this->prf, this->key) ||
			!this->prf->get_bytes(this->prf, this->value, this->value.ptr))
		{
			return FALSE;
		}
	}
	DBG4(DBG_LIB, "HMAC_DRBG K: %B", &this->key);
	DBG4(DBG_LIB, "HMAC_DRBG V: %B", &this->value);

	return TRUE;
}

METHOD(drbg_t, reseed, bool,
	private_drbg_hmac_t *this)
{
	chunk_t seed;
	bool success;

	seed = chunk_alloc(this->strength / BITS_PER_BYTE);
	DBG2(DBG_LIB, "DRBG requests %u bytes of entropy", seed.len);

	if (!this->entropy->get_bytes(this->entropy, seed.len, seed.ptr))
	{
		chunk_free(&seed);
		return FALSE;
	}
	DBG4(DBG_LIB, "reseed: %B", &seed);

	success = update(this, seed);
	chunk_clear(&seed);

	if (!success)
	{
		return FALSE;
	}
	this->reseed_counter = 1;

	return TRUE;
}

METHOD(drbg_t, generate, bool,
	private_drbg_hmac_t *this, uint32_t len, uint8_t *out)
{
	size_t delta;
	chunk_t output;

	if (len > MAX_DRBG_BYTES)
	{
		DBG1(DBG_LIB, "DRBG cannot generate more than %d bytes", MAX_DRBG_BYTES);
		return FALSE;
	}

	if (this->reseed_counter > this->max_requests)
	{
		if (!reseed(this))
		{
			return FALSE;
		}
	}

	DBG2(DBG_LIB, "DRBG generates %u pseudorandom bytes", len);
	if (!out || len == 0)
	{
		return FALSE;
	}
	output = chunk_create(out, len);

	while (len)
	{
		if (!this->prf->get_bytes(this->prf, this->value, this->value.ptr))
		{
			return FALSE;
		}
		delta = min(len, this->value.len);
		memcpy(out, this->value.ptr, delta);
		len -= delta;
		out += delta;
	}
	DBG4(DBG_LIB, "HMAC_DRBG Out: %B", &output);

	if (!update(this, chunk_empty))
	{
		return FALSE;
	}
	this->reseed_counter++;

	return TRUE;
}

METHOD(drbg_t, get_ref, drbg_t*,
	private_drbg_hmac_t *this)
{
	ref_get(&this->ref);
	return &this->public.interface;
}

METHOD(drbg_t, destroy, void,
	private_drbg_hmac_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->entropy);
		this->prf->destroy(this->prf);
		chunk_clear(&this->key);
		chunk_clear(&this->value);
		free(this);
	}
}

/**
 * See header
 */
drbg_hmac_t *drbg_hmac_create(drbg_type_t type, uint32_t strength,
							  rng_t *entropy, chunk_t personalization_str)
{
	private_drbg_hmac_t *this;
	pseudo_random_function_t prf_type = PRF_UNDEFINED;
	size_t out_len, entropy_len;
	uint32_t max_requests;
	chunk_t seed;
	prf_t * prf;
	bool success;

	switch (type)
	{
		case DRBG_HMAC_SHA1:
			prf_type = PRF_HMAC_SHA1;
			break;
		case DRBG_HMAC_SHA256:
			prf_type = PRF_HMAC_SHA2_256;
			break;
		case DRBG_HMAC_SHA384:
			prf_type = PRF_HMAC_SHA2_384;
			break;
		case DRBG_HMAC_SHA512:
			prf_type = PRF_HMAC_SHA2_512;
			break;
		default:
			DBG1(DBG_LIB, "%N not supported", drbg_type_names, type);
			return NULL;
	}

	prf = lib->crypto->create_prf(lib->crypto, prf_type);
	if (!prf)
	{
		DBG1(DBG_LIB, "creation of %N for DRBG failed",
			 pseudo_random_function_names, prf_type);
		return NULL;
	}
	out_len = prf->get_key_size(prf);

	if (strength >  out_len * BITS_PER_BYTE)
	{
		DBG1(DBG_LIB, "%N not sufficient for security strength of %u bits",
			 pseudo_random_function_names, prf_type, strength);
		prf->destroy(prf);
		return NULL;
	}

	max_requests = lib->settings->get_int(lib->settings,
										  "%s.plugins.drbg.max_drbg_requests",
										  MAX_DRBG_REQUESTS, lib->ns);

	INIT(this,
		.public = {
			.interface = {
				.get_type = _get_type,
				.get_strength = _get_strength,
				.reseed = _reseed,
				.generate = _generate,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.type = type,
		.strength = strength,
		.prf = prf,
		.key = chunk_alloc(out_len),
		.value = chunk_alloc(out_len),
		.max_requests = max_requests,
		.reseed_counter = 1,
		.ref = 1,
	);

	memset(this->key.ptr,   0x00, out_len);
	memset(this->value.ptr, 0x01, out_len);

	entropy_len = (strength + strength/2) / BITS_PER_BYTE;
	seed = chunk_alloc(entropy_len + personalization_str.len);
	DBG2(DBG_LIB, "DRBG requests %u bytes of entropy", entropy_len);

	if (!entropy->get_bytes(entropy, entropy_len, seed.ptr))
	{
		chunk_free(&seed);
		destroy(this);
		return NULL;
	}
	memcpy(seed.ptr + entropy_len,
		   personalization_str.ptr, personalization_str.len);
	DBG4(DBG_LIB, "seed: %B", &seed);

	success = update(this, seed);
	chunk_clear(&seed);

	if (!success)
	{
		destroy(this);
		return NULL;
	}

	/* ownership of entropy source is transferred to DRBG */
	this->entropy = entropy;

	return &this->public;
}
