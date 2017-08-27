/*
 * Copyright (C) 2013 Tobias Brunner
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

#include "pkcs12.h"

#include <utils/debug.h>

/**
 * v * ceiling(len/v)
 */
#define PKCS12_LEN(len, v) (((len) + v-1) & ~(v-1))

/**
 * Copy src to dst as many times as possible
 */
static inline void copy_chunk(chunk_t dst, chunk_t src)
{
	size_t i;

	for (i = 0; i < dst.len; i++)
	{
		dst.ptr[i] = src.ptr[i % src.len];
	}
}

/**
 * Treat two chunks as integers in network order and add them together.
 * The result is stored in the first chunk, if the second chunk is longer or the
 * result overflows this is ignored.
 */
static void add_chunks(chunk_t a, chunk_t b)
{
	u_int16_t sum;
	u_int8_t rem = 0;
	ssize_t i, j;

	for (i = a.len - 1, j = b.len -1; i >= 0 && j >= 0; i--, j--)
	{
		sum = a.ptr[i] + b.ptr[j] + rem;
		a.ptr[i] = (u_char)sum;
		rem = sum >> 8;
	}
	for (; i >= 0 && rem; i--)
	{
		sum = a.ptr[i] + rem;
		a.ptr[i] = (u_char)sum;
		rem = sum >> 8;
	}
}

/**
 * Do the actual key derivation with the given hasher, password and id.
 */
static bool derive_key(hash_algorithm_t hash, chunk_t unicode, chunk_t salt,
					   u_int64_t iterations, char id, chunk_t result)
{
	chunk_t out = result, D, S, P = chunk_empty, I, Ai, B, Ij;
	hasher_t *hasher;
	size_t Slen, v, u;
	u_int64_t i;
	bool success = FALSE;

	hasher = lib->crypto->create_hasher(lib->crypto, hash);
	if (!hasher)
	{
		DBG1(DBG_ASN, "  %N hash algorithm not available",
			 hash_algorithm_names, hash);
		return  FALSE;
	}
	switch (hash)
	{
		case HASH_MD2:
		case HASH_MD5:
		case HASH_SHA1:
		case HASH_SHA224:
		case HASH_SHA256:
			v = 64;
			break;
		case HASH_SHA384:
		case HASH_SHA512:
			v = 128;
			break;
		default:
			goto end;
	}
	u = hasher->get_hash_size(hasher);

	D = chunk_alloca(v);
	memset(D.ptr, id, D.len);

	Slen = PKCS12_LEN(salt.len, v);
	I = chunk_alloca(Slen + PKCS12_LEN(unicode.len, v));
	S = chunk_create(I.ptr, Slen);
	P = chunk_create(I.ptr + Slen, I.len - Slen);
	copy_chunk(S, salt);
	copy_chunk(P, unicode);

	Ai = chunk_alloca(u);
	B = chunk_alloca(v);

	while (TRUE)
	{
		if (!hasher->get_hash(hasher, D, NULL) ||
			!hasher->get_hash(hasher, I, Ai.ptr))
		{
			goto end;
		}
		for (i = 1; i < iterations; i++)
		{
			if (!hasher->get_hash(hasher, Ai, Ai.ptr))
			{
				goto end;
			}
		}
		memcpy(out.ptr, Ai.ptr, min(out.len, Ai.len));
		out = chunk_skip(out, Ai.len);
		if (!out.len)
		{
			break;
		}
		copy_chunk(B, Ai);
		/* B = B+1 */
		add_chunks(B, chunk_from_chars(0x01));
		Ij = chunk_create(I.ptr, v);
		for (i = 0; i < I.len; i += v, Ij.ptr += v)
		{	/* Ij = Ij + B + 1 */
			add_chunks(Ij, B);
		}
	}
	success = TRUE;
end:
	hasher->destroy(hasher);
	return success;
}

/*
 * Described in header
 */
bool pkcs12_derive_key(hash_algorithm_t hash, chunk_t password, chunk_t salt,
					   u_int64_t iterations, pkcs12_key_type_t type, chunk_t key)
{
	chunk_t unicode = chunk_empty;
	bool success;
	int i;

	if (password.len)
	{	/* convert the password to UTF-16BE (without BOM) with 0 terminator */
		unicode = chunk_alloca(password.len * 2 + 2);
		for (i = 0; i < password.len; i++)
		{
			unicode.ptr[i * 2] = 0;
			unicode.ptr[i * 2 + 1] = password.ptr[i];
		}
		unicode.ptr[i * 2] = 0;
		unicode.ptr[i * 2 + 1] = 0;
	}

	success = derive_key(hash, unicode, salt, iterations, type, key);
	memwipe(unicode.ptr, unicode.len);
	return success;
}
