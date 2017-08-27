/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
 * Copyright (C) 1990-1992, RSA Data Security, Inc. Created 1990.
 * All rights reserved.
 *
 * Derived from the RSA Data Security, Inc. MD4 Message-Digest Algorithm.
 * Ported to fulfill hasher_t interface.
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

#include "md4_hasher.h"

/*
 * Constants for MD4Transform routine.
 */
#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15

static u_int8_t PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * F, G, H and I are basic MD4 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))

/*
 * ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG and HH are transformations for rounds 1, 2 and 3
 * Rotation is separate from addition to prevent recomputation
 */
#define FF(a, b, c, d, x, s) { \
    (a) += F ((b), (c), (d)) + (x); \
    (a) = ROTATE_LEFT ((a), (s)); \
  }
#define GG(a, b, c, d, x, s) { \
    (a) += G ((b), (c), (d)) + (x) + (u_int32_t)0x5a827999; \
    (a) = ROTATE_LEFT ((a), (s)); \
  }
#define HH(a, b, c, d, x, s) { \
    (a) += H ((b), (c), (d)) + (x) + (u_int32_t)0x6ed9eba1; \
    (a) = ROTATE_LEFT ((a), (s)); \
  }

typedef struct private_md4_hasher_t private_md4_hasher_t;

/**
 * Private data structure with hasing context.
 */
struct private_md4_hasher_t {
	/**
	 * Public interface for this hasher.
	 */
	md4_hasher_t public;

	/*
	 * State of the hasher.
	 */
	u_int32_t state[4];
    u_int32_t count[2];
    u_int8_t buffer[64];
};

#if BYTE_ORDER != LITTLE_ENDIAN

/* Encodes input (u_int32_t) into output (u_int8_t). Assumes len is
 * a multiple of 4.
 */
static void Encode (u_int8_t *output, u_int32_t *input, size_t len)
{
	size_t i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		output[j] = (u_int8_t)(input[i] & 0xff);
		output[j+1] = (u_int8_t)((input[i] >> 8) & 0xff);
		output[j+2] = (u_int8_t)((input[i] >> 16) & 0xff);
		output[j+3] = (u_int8_t)((input[i] >> 24) & 0xff);
	}
}

/* Decodes input (u_int8_t) into output (u_int32_t). Assumes len is
 * a multiple of 4.
 */
static void Decode(u_int32_t *output, u_int8_t *input, size_t len)
{
	size_t i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		output[i] = ((u_int32_t)input[j]) | (((u_int32_t)input[j+1]) << 8) |
		(((u_int32_t)input[j+2]) << 16) | (((u_int32_t)input[j+3]) << 24);
	}
}

#elif BYTE_ORDER == LITTLE_ENDIAN
 #define Encode memcpy
 #define Decode memcpy
#endif

/*
 * MD4 basic transformation. Transforms state based on block.
 */
static void MD4Transform(u_int32_t state[4], u_int8_t block[64])
{
	u_int32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	Decode(x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11); /* 1 */
  FF (d, a, b, c, x[ 1], S12); /* 2 */
  FF (c, d, a, b, x[ 2], S13); /* 3 */
  FF (b, c, d, a, x[ 3], S14); /* 4 */
  FF (a, b, c, d, x[ 4], S11); /* 5 */
  FF (d, a, b, c, x[ 5], S12); /* 6 */
  FF (c, d, a, b, x[ 6], S13); /* 7 */
  FF (b, c, d, a, x[ 7], S14); /* 8 */
  FF (a, b, c, d, x[ 8], S11); /* 9 */
  FF (d, a, b, c, x[ 9], S12); /* 10 */
  FF (c, d, a, b, x[10], S13); /* 11 */
  FF (b, c, d, a, x[11], S14); /* 12 */
  FF (a, b, c, d, x[12], S11); /* 13 */
  FF (d, a, b, c, x[13], S12); /* 14 */
  FF (c, d, a, b, x[14], S13); /* 15 */
  FF (b, c, d, a, x[15], S14); /* 16 */

  /* Round 2 */
  GG (a, b, c, d, x[ 0], S21); /* 17 */
  GG (d, a, b, c, x[ 4], S22); /* 18 */
  GG (c, d, a, b, x[ 8], S23); /* 19 */
  GG (b, c, d, a, x[12], S24); /* 20 */
  GG (a, b, c, d, x[ 1], S21); /* 21 */
  GG (d, a, b, c, x[ 5], S22); /* 22 */
  GG (c, d, a, b, x[ 9], S23); /* 23 */
  GG (b, c, d, a, x[13], S24); /* 24 */
  GG (a, b, c, d, x[ 2], S21); /* 25 */
  GG (d, a, b, c, x[ 6], S22); /* 26 */
  GG (c, d, a, b, x[10], S23); /* 27 */
  GG (b, c, d, a, x[14], S24); /* 28 */
  GG (a, b, c, d, x[ 3], S21); /* 29 */
  GG (d, a, b, c, x[ 7], S22); /* 30 */
  GG (c, d, a, b, x[11], S23); /* 31 */
  GG (b, c, d, a, x[15], S24); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 0], S31); /* 33 */
  HH (d, a, b, c, x[ 8], S32); /* 34 */
  HH (c, d, a, b, x[ 4], S33); /* 35 */
  HH (b, c, d, a, x[12], S34); /* 36 */
  HH (a, b, c, d, x[ 2], S31); /* 37 */
  HH (d, a, b, c, x[10], S32); /* 38 */
  HH (c, d, a, b, x[ 6], S33); /* 39 */
  HH (b, c, d, a, x[14], S34); /* 40 */
  HH (a, b, c, d, x[ 1], S31); /* 41 */
  HH (d, a, b, c, x[ 9], S32); /* 42 */
  HH (c, d, a, b, x[ 5], S33); /* 43 */
  HH (b, c, d, a, x[13], S34); /* 44 */
  HH (a, b, c, d, x[ 3], S31); /* 45 */
  HH (d, a, b, c, x[11], S32); /* 46 */
  HH (c, d, a, b, x[ 7], S33); /* 47 */
  HH (b, c, d, a, x[15], S34); /* 48 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
}

/* MD4 block update operation. Continues an MD4 message-digest
 * operation, processing another message block, and updating the
 * context.
 */
static void MD4Update(private_md4_hasher_t *this, u_int8_t *input, size_t inputLen)
{
	u_int32_t i;
	size_t index, partLen;

	/* Compute number of bytes mod 64 */
	index = (u_int8_t)((this->count[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ((this->count[0] += (inputLen << 3)) < (inputLen << 3))
	{
		this->count[1]++;
	}
	this->count[1] += (inputLen >> 29);

	partLen = 64 - index;

	/* Transform as many times as possible. */
	if (inputLen >= partLen)
	{
		memcpy(&this->buffer[index], input, partLen);
		MD4Transform (this->state, this->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
		{
		MD4Transform (this->state, &input[i]);
		}
		index = 0;
	}
	else
	{
		i = 0;
	}

	/* Buffer remaining input */
	memcpy(&this->buffer[index], &input[i], inputLen-i);
}

/* MD4 finalization. Ends an MD4 message-digest operation, writing the
 * the message digest and zeroizing the context.
 */
static void MD4Final (private_md4_hasher_t *this, u_int8_t digest[16])
{
	u_int8_t bits[8];
	size_t index, padLen;

	/* Save number of bits */
	Encode (bits, this->count, 8);

	/* Pad out to 56 mod 64. */
	index = (size_t)((this->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	MD4Update (this, PADDING, padLen);

	/* Append length (before padding) */
	MD4Update (this, bits, 8);

	if (digest != NULL)			/* Bill Simpson's padding */
	{
		/* store state in digest */
		Encode (digest, this->state, 16);
	}
}

METHOD(hasher_t, reset, bool,
	private_md4_hasher_t *this)
{
	this->state[0] = 0x67452301;
	this->state[1] = 0xefcdab89;
	this->state[2] = 0x98badcfe;
	this->state[3] = 0x10325476;
	this->count[0] = 0;
	this->count[1] = 0;

	return TRUE;
}

METHOD(hasher_t, get_hash, bool,
	private_md4_hasher_t *this, chunk_t chunk, u_int8_t *buffer)
{
	MD4Update(this, chunk.ptr, chunk.len);
	if (buffer != NULL)
	{
		MD4Final(this, buffer);
		reset(this);
	}
	return TRUE;
}

METHOD(hasher_t, allocate_hash, bool,
	private_md4_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	chunk_t allocated_hash;

	MD4Update(this, chunk.ptr, chunk.len);
	if (hash != NULL)
	{
		allocated_hash.ptr = malloc(HASH_SIZE_MD4);
		allocated_hash.len = HASH_SIZE_MD4;

		MD4Final(this, allocated_hash.ptr);
		reset(this);

		*hash = allocated_hash;
	}
	return TRUE;
}

METHOD(hasher_t, get_hash_size, size_t,
	private_md4_hasher_t *this)
{
	return HASH_SIZE_MD4;
}

METHOD(hasher_t, destroy, void,
	private_md4_hasher_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
md4_hasher_t *md4_hasher_create(hash_algorithm_t algo)
{
	private_md4_hasher_t *this;

	if (algo != HASH_MD4)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.hasher_interface = {
				.get_hash = _get_hash,
				.allocate_hash = _allocate_hash,
				.get_hash_size = _get_hash_size,
				.reset = _reset,
				.destroy = _destroy,
			},
		},
	);

	/* initialize */
	reset(this);

	return &(this->public);
}
