/*
 * Copyright (C) 2019 Sean Parkinson, wolfSSL Inc.
 * Copyright (C) 2021 Andreas Steffen, strongSec GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "wolfssl_common.h"
#include "wolfssl_util.h"

#include <utils/debug.h>

#include <wolfssl/wolfcrypt/hash.h>
#include <wolfssl/wolfcrypt/rsa.h>

/*
 * Described in header
 */
bool wolfssl_hash_chunk(int hash_type, chunk_t data, chunk_t *hash)
{
	int ret;

	*hash = chunk_alloc(wc_HashGetDigestSize(hash_type));
	ret = wc_Hash(hash_type, data.ptr, data.len, hash->ptr, hash->len);
	if (ret < 0)
	{
		chunk_free(hash);
		return FALSE;
	}
	return TRUE;
}

/*
 * Described in header
 */
bool wolfssl_mp2chunk(mp_int *mp, chunk_t *chunk)
{
	*chunk = chunk_alloc(mp_unsigned_bin_size(mp));
	if (mp_to_unsigned_bin(mp, chunk->ptr) == 0)
	{
		if (chunk->len && chunk->ptr[0] & 0x80)
		{	/* if MSB is set, prepend a zero to make it non-negative */
			*chunk = chunk_cat("cm", chunk_from_chars(0x00), *chunk);
		}
		return TRUE;
	}
	chunk_free(chunk);
	return FALSE;
}

/*
 * Described in header
 */
bool wolfssl_mp_split(chunk_t chunk, mp_int *a, mp_int *b)
{
	int ret;
	int len;

	if ((chunk.len % 2) == 1)
	{
		return FALSE;
	}

	len = chunk.len / 2;
	ret = mp_read_unsigned_bin(a, chunk.ptr, len);
	if (ret == 0)
	{
		ret = mp_read_unsigned_bin(b, chunk.ptr + len, len);
	}
	return ret == 0;
}

/*
 * Described in header
 */
bool wolfssl_mp_cat(int len, mp_int *a, mp_int *b, chunk_t *chunk)
{
	int ret;
	int sz;

	*chunk = chunk_alloc(len);
	if (b != NULL)
	{
		len /= 2;
	}

	sz = mp_unsigned_bin_size(a);
	memset(chunk->ptr, 0, len - sz);
	ret = mp_to_unsigned_bin(a, chunk->ptr + len - sz);
	if (ret == 0 && b != NULL)
	{
		sz = mp_unsigned_bin_size(b);
		memset(chunk->ptr + len, 0, len - sz);
		ret = mp_to_unsigned_bin(b, chunk->ptr + 2 * len - sz);
	}
	return ret == 0;
}

/*
 * Described in header
 */
bool wolfssl_hash2type(hash_algorithm_t hash, enum wc_HashType *type)
{
	switch (hash)
	{
#ifndef NO_MD5
		case HASH_MD5:
			*type = WC_HASH_TYPE_MD5;
			break;
#endif
#ifndef NO_SHA
		case HASH_SHA1:
			*type = WC_HASH_TYPE_SHA;
			break;
#endif
#ifdef WOLFSSL_SHA224
		case HASH_SHA224:
			*type = WC_HASH_TYPE_SHA224;
			break;
#endif
#ifndef NO_SHA256
		case HASH_SHA256:
			*type = WC_HASH_TYPE_SHA256;
			break;
#endif
#ifdef WOLFSSL_SHA384
		case HASH_SHA384:
			*type = WC_HASH_TYPE_SHA384;
			break;
#endif
#ifdef WOLFSSL_SHA512
		case HASH_SHA512:
			*type = WC_HASH_TYPE_SHA512;
			break;
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_224)
		case HASH_SHA3_224:
			*type = WC_HASH_TYPE_SHA3_224;
			break;
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_256)
		case HASH_SHA3_256:
			*type = WC_HASH_TYPE_SHA3_256;
			break;
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_384)
		case HASH_SHA3_384:
			*type = WC_HASH_TYPE_SHA3_384;
			break;
#endif
#if defined(WOLFSSL_SHA3) && !defined(WOLFSSL_NOSHA3_512)
		case HASH_SHA3_512:
			*type = WC_HASH_TYPE_SHA3_512;
			break;
#endif
		default:
			return FALSE;
	}
	return TRUE;
}

/*
 * Described in header
 */
bool wolfssl_hash2mgf1(hash_algorithm_t hash, int *mgf1)
{
	switch (hash)
	{
#ifndef NO_SHA
		case HASH_SHA1:
			*mgf1 = WC_MGF1SHA1;
			break;
#endif
#ifdef WOLFSSL_SHA224
		case HASH_SHA224:
			*mgf1 = WC_MGF1SHA224;
			break;
#endif
#ifndef NO_SHA256
		case HASH_SHA256:
			*mgf1 = WC_MGF1SHA256;
			break;
#endif
#ifdef WOLFSSL_SHA384
		case HASH_SHA384:
			*mgf1 = WC_MGF1SHA384;
			break;
#endif
#ifdef WOLFSSL_SHA512
		case HASH_SHA512:
			*mgf1 = WC_MGF1SHA512;
			break;
#endif
		default:
			return FALSE;
	}
	return TRUE;
}
