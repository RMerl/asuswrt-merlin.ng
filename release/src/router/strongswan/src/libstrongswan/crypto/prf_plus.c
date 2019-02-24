/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include <string.h>

#include "prf_plus.h"

typedef struct private_prf_plus_t private_prf_plus_t;

/**
 * Private data of an prf_plus_t object.
 *
 */
struct private_prf_plus_t {

	/**
	 * Public interface of prf_plus_t.
	 */
	prf_plus_t public;

	/**
	 * PRF to use.
	 */
	prf_t *prf;

	/**
	 * Initial seed.
	 */
	chunk_t seed;

	/**
	 * Octet which will be appended to the seed, 0 if not used
	 */
	uint8_t counter;

	/**
	 * Already given out bytes in current buffer.
	 */
	size_t used;

	/**
	 * Buffer to store current PRF result.
	 */
	chunk_t buffer;
};

METHOD(prf_plus_t, get_bytes, bool,
	private_prf_plus_t *this, size_t length, uint8_t *buffer)
{
	size_t round, written = 0;

	while (length > 0)
	{
		if (this->buffer.len == this->used)
		{	/* buffer used, get next round */
			if (!this->prf->get_bytes(this->prf, this->buffer, NULL))
			{
				return FALSE;
			}
			if (this->counter)
			{
				if (!this->prf->get_bytes(this->prf, this->seed, NULL) ||
					!this->prf->get_bytes(this->prf,
							chunk_from_thing(this->counter), this->buffer.ptr))
				{
					return FALSE;
				}
				this->counter++;
			}
			else
			{
				if (!this->prf->get_bytes(this->prf, this->seed,
										  this->buffer.ptr))
				{
					return FALSE;
				}
			}
			this->used = 0;
		}
		round = min(length, this->buffer.len - this->used);
		memcpy(buffer + written, this->buffer.ptr + this->used, round);

		length -= round;
		this->used += round;
		written += round;
	}
	return TRUE;
}

METHOD(prf_plus_t, allocate_bytes, bool,
	private_prf_plus_t *this, size_t length, chunk_t *chunk)
{
	if (length)
	{
		*chunk = chunk_alloc(length);
		return get_bytes(this, length, chunk->ptr);
	}
	*chunk = chunk_empty;
	return TRUE;
}

METHOD(prf_plus_t, destroy, void,
	private_prf_plus_t *this)
{
	chunk_clear(&this->buffer);
	chunk_clear(&this->seed);
	free(this);
}

/*
 * Description in header.
 */
prf_plus_t *prf_plus_create(prf_t *prf, bool counter, chunk_t seed)
{
	private_prf_plus_t *this;

	INIT(this,
		.public = {
			.get_bytes = _get_bytes,
			.allocate_bytes = _allocate_bytes,
			.destroy = _destroy,
		},
		.prf = prf,
		.seed = chunk_clone(seed),
		.buffer = chunk_alloc(prf->get_block_size(prf)),
	);

	if (counter)
	{
		this->counter = 0x01;
		if (!this->prf->get_bytes(this->prf, this->seed, NULL) ||
			!this->prf->get_bytes(this->prf, chunk_from_thing(this->counter),
								  this->buffer.ptr))
		{
			destroy(this);
			return NULL;
		}
		this->counter++;
	}
	else
	{
		if (!this->prf->get_bytes(this->prf, this->seed, this->buffer.ptr))
		{
			destroy(this);
			return NULL;
		}
	}

	return &this->public;
}
