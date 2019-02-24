/*
 * Copyright (C) 2013 Andreas Steffen
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

#include "ntru_drbg.h"

#include <utils/debug.h>
#include <utils/test.h>

#define	MAX_STRENGTH_BITS	256
#define MAX_DRBG_REQUESTS	0xfffffffe

typedef struct private_ntru_drbg_t private_ntru_drbg_t;

/**
 * Private data of an ntru_drbg_t object.
 */
struct private_ntru_drbg_t {
	/**
	 * Public ntru_drbg_t interface.
	 */
	ntru_drbg_t public;

	/**
	 * Security strength in bits of the DRBG
	 */
	uint32_t strength;

	/**
	 * Number of requests for pseudorandom bits
	 */
	uint32_t reseed_counter;

	/**
	 * Maximum number of requests for pseudorandom bits
	 */
	uint32_t max_requests;

	/**
	 * True entropy source
	 */
	rng_t *entropy;

	/**
	 * HMAC-SHA256
	 */
	signer_t *hmac;

	/**
	 * Internal state of HMAC-SHA256: key
	 */
	chunk_t key;

	/**
	 * Internal state of HMAC-SHA256: value
	 */
	chunk_t value;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * Update the internal state of the HMAC_DRBG
 */
static bool update(private_ntru_drbg_t *this, chunk_t data)
{
	chunk_t ch_00 = chunk_from_chars(0x00);
	chunk_t ch_01 = chunk_from_chars(0x01);

	if (!this->hmac->set_key(this->hmac, this->key) ||
		!this->hmac->get_signature(this->hmac, this->value, NULL) ||
	    !this->hmac->get_signature(this->hmac, ch_00, NULL) ||
	    !this->hmac->get_signature(this->hmac, data, this->key.ptr) ||
		!this->hmac->set_key(this->hmac, this->key) ||
	    !this->hmac->get_signature(this->hmac, this->value,
											   this->value.ptr))
	{
		return FALSE;
	}

	if (data.len > 0)
	{
		if (!this->hmac->set_key(this->hmac, this->key) ||
			!this->hmac->get_signature(this->hmac, this->value, NULL) ||
			!this->hmac->get_signature(this->hmac, ch_01, NULL) ||
			!this->hmac->get_signature(this->hmac, data, this->key.ptr) ||
			!this->hmac->set_key(this->hmac, this->key) ||
			!this->hmac->get_signature(this->hmac, this->value,
												   this->value.ptr))
		{
			return FALSE;
		}
	}
	DBG4(DBG_LIB, "HMAC_DRBG V: %B", &this->value);
	DBG4(DBG_LIB, "HMAC_DRBG K: %B", &this->key);

	return TRUE;
}

METHOD(ntru_drbg_t, get_strength, uint32_t,
	private_ntru_drbg_t *this)
{
	return this->strength;
}

METHOD(ntru_drbg_t, reseed, bool,
	private_ntru_drbg_t *this)
{
	chunk_t seed;

	seed = chunk_alloc(this->strength / BITS_PER_BYTE);
	DBG2(DBG_LIB, "DRBG requests %u bytes of entropy", seed.len);

	if (!this->entropy->get_bytes(this->entropy, seed.len, seed.ptr))
	{
		chunk_free(&seed);
		return FALSE;
	}
	if (!update(this, seed))
	{
		chunk_free(&seed);
		return FALSE;
	}
	chunk_clear(&seed);
	this->reseed_counter = 1;

	return TRUE;
}

METHOD(ntru_drbg_t, generate, bool,
	private_ntru_drbg_t *this, uint32_t strength, uint32_t len, uint8_t *out)
{
	size_t delta;
	chunk_t output;

	DBG2(DBG_LIB, "DRBG generates %u pseudorandom bytes", len);
	if (!out || len == 0)
	{
		return FALSE;
	}
	output = chunk_create(out, len);

	if (this->reseed_counter > this->max_requests)
	{
		if (!reseed(this))
		{
			return FALSE;
		}
	}
	while (len)
	{
		if (!this->hmac->get_signature(this->hmac, this->value,
												   this->value.ptr))
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

METHOD(ntru_drbg_t, get_ref, ntru_drbg_t*,
	private_ntru_drbg_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(ntru_drbg_t, destroy, void,
	private_ntru_drbg_t *this)
{
	if (ref_put(&this->ref))
	{
		this->hmac->destroy(this->hmac);
		chunk_clear(&this->key);
		chunk_clear(&this->value);
		free(this);
	}
}

/*
 * Described in header.
 */
ntru_drbg_t *ntru_drbg_create(uint32_t strength, chunk_t pers_str,
							  rng_t *entropy)
{
	private_ntru_drbg_t *this;
	chunk_t seed;
	signer_t *hmac;
	size_t entropy_len;
	uint32_t max_requests;

	if (strength > MAX_STRENGTH_BITS)
	{
		return NULL;
	}
	if (strength <= 112)
	{
		strength = 112;
	}
	else if (strength <= 128)
	{
		strength = 128;
	}
	else if (strength <= 192)
	{
		strength = 192;
	}
	else
	{
		strength = 256;
	}

	hmac = lib->crypto->create_signer(lib->crypto, AUTH_HMAC_SHA2_256_256);
	if (!hmac)
	{
		DBG1(DBG_LIB, "could not instantiate HMAC-SHA256");
		return NULL;
	}

	max_requests = lib->settings->get_int(lib->settings,
										  "%s.plugins.ntru.max_drbg_requests",
										  MAX_DRBG_REQUESTS, lib->ns);

	INIT(this,
		.public = {
			.get_strength = _get_strength,
			.reseed = _reseed,
			.generate = _generate,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.strength = strength,
		.entropy = entropy,
		.hmac = hmac,
		.key = chunk_alloc(hmac->get_key_size(hmac)),
		.value = chunk_alloc(hmac->get_block_size(hmac)),
		.max_requests = max_requests,
		.reseed_counter = 1,
		.ref = 1,
	);

	memset(this->key.ptr, 0x00, this->key.len);
	memset(this->value.ptr, 0x01, this->value.len);

	entropy_len = (strength + strength/2) / BITS_PER_BYTE;
	seed = chunk_alloc(entropy_len + pers_str.len);
	DBG2(DBG_LIB, "DRBG requests %u bytes of entropy", entropy_len);

	if (!this->entropy->get_bytes(this->entropy, entropy_len, seed.ptr))
	{
		chunk_free(&seed);
		destroy(this);
		return NULL;
	}
	memcpy(seed.ptr + entropy_len, pers_str.ptr, pers_str.len);
	DBG4(DBG_LIB, "seed: %B", &seed);

	if (!update(this, seed))
	{
		chunk_free(&seed);
		destroy(this);
		return NULL;
	}
	chunk_clear(&seed);

	return &this->public;
}

EXPORT_FUNCTION_FOR_TESTS(ntru, ntru_drbg_create);
