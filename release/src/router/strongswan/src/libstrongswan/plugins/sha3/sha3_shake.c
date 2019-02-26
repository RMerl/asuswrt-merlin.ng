/*
 * Copyright (C) 2016 Andreas Steffen
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

#include "sha3_shake.h"
#include "sha3_keccak.h"

typedef struct private_sha3_shake_t private_sha3_shake_t;


/**
 * Private data structure with hashing context for SHA-3
 */
struct private_sha3_shake_t {

	/**
	 * Public interface for this hasher.
	 */
	sha3_shake_t public;

	/**
	 * XOF algorithm to be used (XOF_SHAKE_128 or XOF_SHAKE_256)
	 */
	ext_out_function_t algorithm;

	/**
	 * SHA-3 Keccak state
	 */
	sha3_keccak_t *keccak;

	/**
	 * Capacity in bytes of the SHA-3 Keccak state
	 */
	u_int capacity;

};

METHOD(xof_t, get_type, ext_out_function_t,
	private_sha3_shake_t *this)
{
	return this->algorithm;
}

METHOD(xof_t, get_bytes, bool,
	private_sha3_shake_t *this, size_t out_len, uint8_t *buffer)
{
	this->keccak->squeeze(this->keccak, out_len, buffer);
	return TRUE;
}

METHOD(xof_t, allocate_bytes, bool,
	private_sha3_shake_t *this, size_t out_len, chunk_t *chunk)
{
	*chunk = chunk_alloc(out_len);
	this->keccak->squeeze(this->keccak, out_len, chunk->ptr);
	return TRUE;
}

METHOD(xof_t, get_block_size, size_t,
	private_sha3_shake_t *this)
{
	return this->keccak->get_rate(this->keccak);
}

METHOD(xof_t, get_seed_size, size_t,
	private_sha3_shake_t *this)
{
	return this->capacity;
}

METHOD(xof_t, set_seed, bool,
	private_sha3_shake_t *this, chunk_t seed)
{
	this->keccak->reset(this->keccak);
	this->keccak->absorb(this->keccak, seed);
	this->keccak->finalize(this->keccak);
	return TRUE;
}


METHOD(xof_t, destroy, void,
	private_sha3_shake_t *this)
{
	this->keccak->destroy(this->keccak);
	free(this);
}

/*
 * Described in header.
 */
sha3_shake_t* sha3_shake_create(ext_out_function_t algorithm)
{
	private_sha3_shake_t *this;
	u_int capacity = 0;

	switch (algorithm)
	{
		case XOF_SHAKE_128:
			capacity = 32;
			break;
		case XOF_SHAKE_256:
			capacity = 64;
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.xof_interface = {
				.get_type = _get_type,
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.get_block_size = _get_block_size,
				.get_seed_size = _get_seed_size,
				.set_seed = _set_seed,
				.destroy = _destroy,
			},
		},
		.algorithm = algorithm,
		.capacity = capacity,
	);

	this->keccak = sha3_keccak_create(capacity, 0x1f);
	if (!this->keccak)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}
