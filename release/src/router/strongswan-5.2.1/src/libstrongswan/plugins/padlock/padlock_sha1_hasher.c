/*
 * Copyright (C) 2008 Thomas Kallenberg
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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
#include <arpa/inet.h>
#include <byteswap.h>

#include "padlock_sha1_hasher.h"

#define PADLOCK_ALIGN __attribute__ ((__aligned__(16)))

typedef struct private_padlock_sha1_hasher_t private_padlock_sha1_hasher_t;

/**
 * Private data structure with hasing context.
 */
struct private_padlock_sha1_hasher_t {
	/**
	 * Public interface for this hasher.
	 */
	padlock_sha1_hasher_t public;

	/**
	 * data collected to hash
	 */
	chunk_t data;
};

/**
 * Invoke the actual padlock sha1() operation
 */
static void padlock_sha1(int len, u_char *in, u_char *out)
{
	/* rep xsha1 */
	asm volatile (
		".byte 0xf3, 0x0f, 0xa6, 0xc8"
		: "+S"(in), "+D"(out)
		: "c"(len), "a"(0));
}

/**
 * sha1() a buffer of data into digest
 */
static void sha1(chunk_t data, u_int32_t *digest)
{
	u_int32_t hash[128] PADLOCK_ALIGN;

	hash[0] = 0x67452301;
	hash[1] = 0xefcdab89;
	hash[2] = 0x98badcfe;
	hash[3] = 0x10325476;
	hash[4] = 0xc3d2e1f0;

	padlock_sha1(data.len, data.ptr, (u_char*)hash);

	digest[0] = bswap_32(hash[0]);
	digest[1] = bswap_32(hash[1]);
	digest[2] = bswap_32(hash[2]);
	digest[3] = bswap_32(hash[3]);
	digest[4] = bswap_32(hash[4]);
}

/**
 * append data to the to-be-hashed buffer
 */
static void append_data(private_padlock_sha1_hasher_t *this, chunk_t data)
{
	this->data.ptr = realloc(this->data.ptr, this->data.len + data.len);
	memcpy(this->data.ptr + this->data.len, data.ptr, data.len);
	this->data.len += data.len;
}

METHOD(hasher_t, reset, bool,
	private_padlock_sha1_hasher_t *this)
{
	chunk_free(&this->data);
	return TRUE;
}

METHOD(hasher_t, get_hash, bool,
	private_padlock_sha1_hasher_t *this, chunk_t chunk, u_int8_t *hash)
{
	if (hash)
	{
		if (this->data.len)
		{
			append_data(this, chunk);
			sha1(this->data, (u_int32_t*)hash);
		}
		else
		{   /* hash directly if no previous data found */
			sha1(chunk, (u_int32_t*)hash);
		}
		reset(this);
	}
	else
	{
		append_data(this, chunk);
	}
	return TRUE;
}

METHOD(hasher_t, allocate_hash, bool,
	private_padlock_sha1_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	if (hash)
	{
		*hash = chunk_alloc(HASH_SIZE_SHA1);
		return get_hash(this, chunk, hash->ptr);
	}
	return get_hash(this, chunk, NULL);
}

METHOD(hasher_t, get_hash_size, size_t,
	private_padlock_sha1_hasher_t *this)
{
	return HASH_SIZE_SHA1;
}

METHOD(hasher_t, destroy, void,
	private_padlock_sha1_hasher_t *this)
{
	free(this->data.ptr);
	free(this);
}

/*
 * Described in header.
 */
padlock_sha1_hasher_t *padlock_sha1_hasher_create(hash_algorithm_t algo)
{
	private_padlock_sha1_hasher_t *this;

	if (algo != HASH_SHA1)
	{
		return NULL;
	}
	INIT(this,
		.public = {
			.hasher = {
				.get_hash = _get_hash,
				.allocate_hash = _allocate_hash,
				.get_hash_size = _get_hash_size,
				.reset = _reset,
				.destroy = _destroy,
			},
		},
	);
	return &this->public;
}
