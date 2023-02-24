/*
 * Copyright (C) 2016-2019 Andreas Steffen
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

#include "drbg_ctr.h"

#define MAX_DRBG_REQUESTS	0xfffffffe	/* 2^32 - 2 */
#define MAX_DRBG_BYTES		0x00010000	/* 2^19 bits = 2^16 bytes */

typedef struct private_drbg_ctr_t private_drbg_ctr_t;

/**
 * Private data of an drbg_ctr_t object.
 */
struct private_drbg_ctr_t {

	/**
	 * Public drbg_ctr_t interface.
	 */
	drbg_ctr_t public;

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
	 * Block cipher in counter mode used by the DRBG
	 */
	crypter_t *crypter;

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
	private_drbg_ctr_t *this)
{
	return this->type;
}

METHOD(drbg_t, get_strength, uint32_t,
	private_drbg_ctr_t *this)
{
	return this->strength;
}

static bool encrypt_ctr(private_drbg_ctr_t *this, chunk_t out)
{
	chunk_t bl = chunk_alloca(this->value.len);
	chunk_t block;
	size_t delta, pos = 0;

	if (!this->crypter->set_key(this->crypter, this->key))
	{
		return FALSE;
	}

	while (pos < out.len)
	{
		/* Increment counter by one */
		chunk_increment(this->value);

		/* Copy current counter to input block */
		delta = out.len - pos;
		block = (delta < this->value.len) ?
					 bl : chunk_create(out.ptr + pos, this->value.len);
		memcpy(block.ptr, this->value.ptr, this->value.len);

		/* ECB encryption */
		if (!this->crypter->encrypt(this->crypter, block, chunk_empty, NULL))
		{
			return FALSE;
		}

		/* Partial output block at the end? */
		if (delta < this->value.len)
		{
			memcpy(out.ptr + pos, block.ptr, delta);
		}
		pos += this->value.len;
	}

	return TRUE;
}

/**
 * Update the internal state of the CTR_DRBG
 */
static bool update(private_drbg_ctr_t *this, chunk_t data)
{
	chunk_t temp;

	if (data.len && data.len != (this->key.len + this->value.len))
	{
		return FALSE;
	}
	temp = chunk_alloca(this->key.len + this->value.len);

	if (!encrypt_ctr(this, temp))
	{
		return FALSE;
	}
	/* Apply data */
	memxor(temp.ptr, data.ptr, data.len);

	/* Copy new key and value */
	memcpy(this->key.ptr, temp.ptr, this->key.len);
	memcpy(this->value.ptr, temp.ptr + this->key.len, this->value.len);
	memwipe(temp.ptr, temp.len);
	DBG4(DBG_LIB, "CTR_DRBG K: %B", &this->key);
	DBG4(DBG_LIB, "CTR_DRBG V: %B", &this->value);

	return TRUE;
}

METHOD(drbg_t, reseed, bool,
	private_drbg_ctr_t *this)
{
	chunk_t seed;
	bool success;

	seed = chunk_alloc(this->key.len + this->value.len);
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
	private_drbg_ctr_t *this, uint32_t len, uint8_t *out)
{
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

	if (!encrypt_ctr(this, output))
	{
		return FALSE;
	}
	DBG4(DBG_LIB, "CTR_DRBG Out: %B", &output);

	if (!update(this, chunk_empty))
	{
		return FALSE;
	}
	this->reseed_counter++;

	return TRUE;
}

METHOD(drbg_t, get_ref, drbg_t*,
	private_drbg_ctr_t *this)
{
	ref_get(&this->ref);
	return &this->public.interface;
}

METHOD(drbg_t, destroy, void,
	private_drbg_ctr_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->entropy);
		this->crypter->destroy(this->crypter);
		chunk_clear(&this->key);
		chunk_clear(&this->value);
		free(this);
	}
}

/**
 * See header
 */
drbg_ctr_t *drbg_ctr_create(drbg_type_t type, uint32_t strength,
							rng_t *entropy, chunk_t personalization_str)
{
	private_drbg_ctr_t *this;
	encryption_algorithm_t crypter_type;
	crypter_t *crypter;
	chunk_t seed;
	size_t key_len, out_len, seed_len;
	uint32_t max_requests;
	bool success;

	switch (type)
	{
		case DRBG_CTR_AES128:
			crypter_type = ENCR_AES_ECB;
			key_len = 16;
			break;
		case DRBG_CTR_AES192:
			crypter_type = ENCR_AES_ECB;
			key_len = 24;
			break;
		case DRBG_CTR_AES256:
			crypter_type = ENCR_AES_ECB;
			key_len = 32;
			break;
		default:
			DBG1(DBG_LIB, "%N not supported", drbg_type_names, type);
			return NULL;
	}

	if (strength >  key_len * BITS_PER_BYTE)
	{
		DBG1(DBG_LIB, "%d bit block encryption key not sufficient for security "
			 "strength of %u bits", key_len * BITS_PER_BYTE, strength);
		return NULL;
	}

	crypter = lib->crypto->create_crypter(lib->crypto, crypter_type, key_len);
	if (!crypter)
	{
		DBG1(DBG_LIB, "creation of %N for DRBG failed",
			 encryption_algorithm_names, crypter_type);
		return NULL;
	}
	out_len = crypter->get_block_size(crypter);
	seed_len = key_len + out_len;

	if (personalization_str.len > seed_len)
	{
		DBG1(DBG_LIB, "personalization string length of %d bytes is larger "
			 "than seed length of %u bytes", personalization_str.len, seed_len);
		crypter->destroy(crypter);
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
		.crypter = crypter,
		.key = chunk_alloc(key_len),
		.value = chunk_alloc(out_len),
		.max_requests = max_requests,
		.reseed_counter = 1,
		.ref = 1,
	);

	memset(this->key.ptr,   0x00, key_len);
	memset(this->value.ptr, 0x00, out_len);

	seed = chunk_alloc(seed_len);
	DBG2(DBG_LIB, "DRBG requests %u bytes of entropy", seed_len);

	if (!entropy->get_bytes(entropy, seed.len, seed.ptr))
	{
		chunk_free(&seed);
		destroy(this);
		return NULL;
	}
	memxor(seed.ptr, personalization_str.ptr, personalization_str.len);
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
