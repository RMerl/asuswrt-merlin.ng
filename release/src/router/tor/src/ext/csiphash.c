/* <MIT License>
 Copyright (c) 2013-2014  Marek Majkowski <marek@popcount.org>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 </MIT License>

 Original location:
    https://github.com/majek/csiphash/

 Solution inspired by code from:
    Samuel Neves (supercop/crypto_auth/siphash24/little)
    djb (supercop/crypto_auth/siphash24/little2)
    Jean-Philippe Aumasson (https://131002.net/siphash/siphash24.c)
*/

#include "lib/cc/torint.h"
#include "lib/err/torerr.h"

#include "ext/siphash.h"
#include <string.h>
#include <stdlib.h>
#include "ext/byteorder.h"

#define ROTATE(x, b) (uint64_t)( ((x) << (b)) | ( (x) >> (64 - (b))) )

#define HALF_ROUND(a,b,c,d,s,t)			\
	a += b; c += d;				\
	b = ROTATE(b, s) ^ a;			\
	d = ROTATE(d, t) ^ c;			\
	a = ROTATE(a, 32);

#define DOUBLE_ROUND(v0,v1,v2,v3)		\
	HALF_ROUND(v0,v1,v2,v3,13,16);		\
	HALF_ROUND(v2,v1,v0,v3,17,21);		\
	HALF_ROUND(v0,v1,v2,v3,13,16);		\
	HALF_ROUND(v2,v1,v0,v3,17,21);

#if 0
/* This does not seem to save very much runtime in the fast case, and it's
 * potentially a big loss in the slow case where we're misaligned and we cross
 * a cache line. */
#if (defined(__i386) || defined(__i386__) || defined(_M_IX86) ||	\
     defined(__x86_64) || defined(__x86_64__) ||			\
     defined(_M_AMD64) || defined(_M_X64) || defined(__INTEL__))
#   define UNALIGNED_OK 1
#endif
#endif

uint64_t siphash24(const void *src, unsigned long src_sz, const struct sipkey *key) {
	const uint8_t *m = src;
	uint64_t k0 = key->k0;
	uint64_t k1 = key->k1;
	uint64_t last7 = (uint64_t)(src_sz & 0xff) << 56;
	size_t i, blocks;

	uint64_t v0 = k0 ^ 0x736f6d6570736575ULL;
	uint64_t v1 = k1 ^ 0x646f72616e646f6dULL;
	uint64_t v2 = k0 ^ 0x6c7967656e657261ULL;
	uint64_t v3 = k1 ^ 0x7465646279746573ULL;

	for (i = 0, blocks = (src_sz & ~7); i < blocks; i+= 8) {
#ifdef UNALIGNED_OK
		uint64_t mi = _le64toh(*(m + i));
#else
		uint64_t mi;
		memcpy(&mi, m + i, 8);
		mi = _le64toh(mi);
#endif
		v3 ^= mi;
		DOUBLE_ROUND(v0,v1,v2,v3);
		v0 ^= mi;
	}

#ifdef __COVERITY__
	{
		uint64_t mi = 0;
		memcpy(&mi, m+i, (src_sz-blocks));
		last7 = _le64toh(mi) | (uint64_t)(src_sz & 0xff) << 56;
	}
#else
	switch (src_sz - blocks) {
		case 7: last7 |= (uint64_t)m[i + 6] << 48; FALLTHROUGH;
		case 6: last7 |= (uint64_t)m[i + 5] << 40; FALLTHROUGH;
		case 5:	last7 |= (uint64_t)m[i + 4] << 32; FALLTHROUGH;
		case 4: last7 |= (uint64_t)m[i + 3] << 24; FALLTHROUGH;
		case 3:	last7 |= (uint64_t)m[i + 2] << 16; FALLTHROUGH;
		case 2:	last7 |= (uint64_t)m[i + 1] <<  8; FALLTHROUGH;
		case 1: last7 |= (uint64_t)m[i + 0]      ; FALLTHROUGH;
		case 0:
		default:;
	}
#endif
	v3 ^= last7;
	DOUBLE_ROUND(v0,v1,v2,v3);
	v0 ^= last7;
	v2 ^= 0xff;
	DOUBLE_ROUND(v0,v1,v2,v3);
	DOUBLE_ROUND(v0,v1,v2,v3);
	return v0 ^ v1 ^ v2 ^ v3;
}


static int the_siphash_key_is_set = 0;
static struct sipkey the_siphash_key;

uint64_t siphash24g(const void *src, unsigned long src_sz) {
	raw_assert(the_siphash_key_is_set);
	return siphash24(src, src_sz, &the_siphash_key);
}

void siphash_set_global_key(const struct sipkey *key)
{
	raw_assert(! the_siphash_key_is_set);
	the_siphash_key.k0 = key->k0;
	the_siphash_key.k1 = key->k1;
	the_siphash_key_is_set = 1;
}

void siphash_unset_global_key(void)
{
	the_siphash_key_is_set = 0;
	memset(&the_siphash_key, 0, sizeof(the_siphash_key));
}
