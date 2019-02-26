/*
 * Copyright (C) 2008 Martin Willi
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

#include "padlock_rng.h"

typedef struct private_padlock_rng_t private_padlock_rng_t;
typedef enum padlock_quality_factor_t padlock_quality_factor_t;

/**
 * Padlock RNG quality factors
 */
enum padlock_quality_factor_t {
	/* Lowest quality: Reads 8 bytes */
	PADLOCK_QF0 = 0x00,
	/* Medium quality: Reads 4 bytes */
	PADLOCK_QF1 = 0x01,
	/* Better quality: Reads 2 bytes */
	PADLOCK_QF2 = 0x10,
	/* Highest quality: Reads 1 byte */
	PADLOCK_QF3 = 0x11,
};

/**
 * Private data of an padlock_rng_t object.
 */
struct private_padlock_rng_t {

	/**
	 * Public padlock_rng_t interface.
	 */
	padlock_rng_t public;

	/**
	 * Padlock quality factor
	 */
	padlock_quality_factor_t quality;
};

/**
 * Get bytes from Padlock RNG. buf should have space for (len + 7)
 */
static void rng(char *buf, int len, int quality)
{
	while (len > 0)
	{
		int status;

		/* run XSTORE until we have all bytes needed. We do not use REP, as
		 * this should not be performance critical and it's easier this way. */
		asm volatile (
			".byte 0x0F,0xA7,0xC0 \n\t"
			: "=D"(buf), "=a"(status)
			: "d"(quality), "D"(buf));

		/* bits[0..4] of status word contains the number of bytes read */
		len -= status & 0x1F;
	}
}

METHOD(rng_t, allocate_bytes, bool,
	private_padlock_rng_t *this, size_t bytes, chunk_t *chunk)
{
	chunk->len = bytes;
	/* padlock requires some additional bytes */
	chunk->ptr = malloc(bytes + 7);

	rng(chunk->ptr, chunk->len, this->quality);
	return TRUE;
}

METHOD(rng_t, get_bytes, bool,
	private_padlock_rng_t *this, size_t bytes, uint8_t *buffer)
{
	chunk_t chunk;

	/* Padlock needs a larger buffer than "bytes", we need a new buffer */
	allocate_bytes(this, bytes, &chunk);
	memcpy(buffer, chunk.ptr, bytes);
	chunk_clear(&chunk);
	return TRUE;
}

METHOD(rng_t, destroy, void,
	private_padlock_rng_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
padlock_rng_t *padlock_rng_create(rng_quality_t quality)
{
	private_padlock_rng_t *this;

	INIT(this,
		.public = {
			.rng = {
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.destroy = _destroy,
			},
		},
	);

	/* map RNG quality to Padlock quality factor */
	switch (quality)
	{
		case RNG_WEAK:
			this->quality = PADLOCK_QF0;
			break;
		case RNG_STRONG:
			this->quality = PADLOCK_QF1;
			break;
		case RNG_TRUE:
			this->quality = PADLOCK_QF3;
			break;
		default:
			free(this);
			return NULL;
	}
	return &this->public;
}

