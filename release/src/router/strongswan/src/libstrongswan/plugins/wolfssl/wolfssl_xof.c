/*
 * Copyright (C) 2021 Andreas Steffen, strongSec GmbH
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

#include <wolfssl/options.h>

#ifdef WOLFSSL_SHAKE256

#include <wolfssl/wolfcrypt/sha3.h>

#include "wolfssl_xof.h"

#define KECCAK_STATE_SIZE 200   /* 1600 bits */
#define SHAKE256_CAPACITY  64   /*  512 bits */

typedef struct private_xof_t private_xof_t;

/**
 * Private data
 */
struct private_xof_t {

	/**
	 * Public interface.
	 */
	xof_t public;

	/**
	 * Internal context
	 */
	wc_Shake shake;

	/**
	 * Current seed
	 */
	chunk_t seed;

	/**
	 * Offset into generated data
	 */
	size_t offset;
};

METHOD(xof_t, get_type, ext_out_function_t,
	private_xof_t *this)
{
	return XOF_SHAKE_256;
}

METHOD(xof_t, get_bytes, bool,
	private_xof_t *this, size_t out_len, uint8_t *buffer)
{
	bool success = FALSE;
	chunk_t data;

	/* we can call wc_Shake256_Final() only once, so to support an arbitrary
	 * number of calls to get_bytes(), we request all the data we already
	 * requested previously and just ignore what we already handed out */
	if (wc_Shake256_Update(&this->shake, this->seed.ptr, this->seed.len) == 0)
	{
		data = chunk_alloc(out_len + this->offset);
		if (wc_Shake256_Final(&this->shake, data.ptr, data.len) == 0)
		{
			memcpy(buffer, data.ptr + this->offset, out_len);
			this->offset += out_len;
			success = TRUE;
		}
		chunk_clear(&data);
	}
	return success;
}

METHOD(xof_t, allocate_bytes, bool,
	private_xof_t *this, size_t out_len, chunk_t *chunk)
{
	*chunk = chunk_alloc(out_len);
	return get_bytes(this, out_len, chunk->ptr);
}

METHOD(xof_t, get_block_size, size_t,
	private_xof_t *this)
{
	return KECCAK_STATE_SIZE - SHAKE256_CAPACITY;
}

METHOD(xof_t, get_seed_size, size_t,
	private_xof_t *this)
{
	return SHAKE256_CAPACITY;
}

METHOD(xof_t, set_seed, bool,
	private_xof_t *this, chunk_t seed)
{
	chunk_clear(&this->seed);
	this->seed = chunk_clone(seed);
	this->offset = 0;
	return TRUE;
}

METHOD(xof_t, destroy, void,
	private_xof_t *this)
{
	wc_Shake256_Free(&this->shake);
	chunk_clear(&this->seed);
	free(this);
}

/*
 * Described in header
 */
xof_t *wolfssl_xof_create(ext_out_function_t algorithm)
{
	private_xof_t *this;

	if (algorithm != XOF_SHAKE_256)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.get_type = _get_type,
			.get_bytes = _get_bytes,
			.allocate_bytes = _allocate_bytes,
			.get_block_size = _get_block_size,
			.get_seed_size = _get_seed_size,
			.set_seed = _set_seed,
			.destroy = _destroy,
		},
	);

	if (wc_InitShake256(&this->shake, NULL, 0) != 0)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}

#endif /* WOLFSSL_SHAKE256 */
