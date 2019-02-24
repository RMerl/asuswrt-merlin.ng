/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
 *
 * Based on public domain code by Andrew Moon and Daniel J. Bernstein.
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

#include "chapoly_drv_ssse3.h"

#ifdef __SSSE3__

#include <utils/cpu_feature.h>

#include <tmmintrin.h>

#define CHACHA_DOUBLEROUNDS 10

typedef struct private_chapoly_drv_ssse3_t private_chapoly_drv_ssse3_t;

/**
 * Private data of an chapoly_drv_ssse3_t object.
 */
struct private_chapoly_drv_ssse3_t {

	/**
	 * Public chapoly_drv_ssse3_t interface.
	 */
	chapoly_drv_t public;

	/**
	 * ChaCha20 state matrix, as 128-bit vectors
	 */
	__m128i m[4];

	/**
	 * Poly1305 update key
	 */
	uint32_t r[5];

	/**
	 * Poly1305 update key r^2
	 */
	uint32_t u[5];

	/**
	 * Poly1305 state
	 */
	uint32_t h[5];

	/**
	 * Poly1305 finalize key
	 */
	uint32_t s[4];
};

/**
 * Read a 32-bit integer from an unaligned address
 */
static inline uint32_t ru32(void *p)
{
	uint32_t ret;

	memcpy(&ret, p, sizeof(ret));
	return ret;
}

/**
 * Write a 32-bit word to an unaligned address
 */
static inline void wu32(void *p, uint32_t v)
{
	memcpy(p, &v, sizeof(v));
}

/**
 * Shift a 64-bit unsigned integer v right by n bits, clamp to 32 bit
*/
static inline uint32_t sr(uint64_t v, u_char n)
{
	return v >> n;
}

/**
 * AND two values, using a native integer size >= sizeof(uint32_t)
 */
static inline u_long and(u_long v, u_long mask)
{
	return v & mask;
}

/**
 * r = shuffle(a ^ b, s)
 */
static inline __m128i sfflxor32(__m128i a, __m128i b, __m128i s)
{
	return _mm_shuffle_epi8(_mm_xor_si128(a, b), s);
}

/**
 * r = rotl32(a ^ b, r)
 */
static inline __m128i rotlxor32(__m128i a, __m128i b, u_char r)
{
	a = _mm_xor_si128(a, b);
	return _mm_or_si128(_mm_slli_epi32(a, r), _mm_srli_epi32(a, 32 - r));
}

/**
 * XOR a Chacha20 keystream block into data, increment counter
 */
static void chacha_block_xor(private_chapoly_drv_ssse3_t *this, void *data)
{
	__m128i x0, x1, x2, x3, r8, r16, *out = data;
	u_int i;

	r8  = _mm_set_epi8(14, 13, 12, 15, 10, 9, 8, 11, 6, 5, 4, 7, 2, 1, 0, 3);
	r16 = _mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);

	x0 = this->m[0];
	x1 = this->m[1];
	x2 = this->m[2];
	x3 = this->m[3];

	for (i = 0 ; i < CHACHA_DOUBLEROUNDS; i++)
	{
		x0 = _mm_add_epi32(x0, x1);
		x3 = sfflxor32(x3, x0, r16);

		x2 = _mm_add_epi32(x2, x3);
		x1 = rotlxor32(x1, x2, 12);

		x0 = _mm_add_epi32(x0, x1);
		x3 = sfflxor32(x3, x0, r8);

		x2 = _mm_add_epi32(x2, x3);
		x1 = rotlxor32(x1, x2, 7);

		x1 = _mm_shuffle_epi32(x1, _MM_SHUFFLE(0, 3, 2, 1));
		x2 = _mm_shuffle_epi32(x2, _MM_SHUFFLE(1, 0, 3, 2));
		x3 = _mm_shuffle_epi32(x3, _MM_SHUFFLE(2, 1, 0, 3));

		x0 = _mm_add_epi32(x0, x1);
		x3 = sfflxor32(x3, x0, r16);

		x2 = _mm_add_epi32(x2, x3);
		x1 = rotlxor32(x1, x2, 12);

		x0 = _mm_add_epi32(x0, x1);
		x3 = sfflxor32(x3, x0, r8);

		x2 = _mm_add_epi32(x2, x3);
		x1 = rotlxor32(x1, x2, 7);

		x1 = _mm_shuffle_epi32(x1, _MM_SHUFFLE(2, 1, 0, 3));
		x2 = _mm_shuffle_epi32(x2, _MM_SHUFFLE(1, 0, 3, 2));
		x3 = _mm_shuffle_epi32(x3, _MM_SHUFFLE(0, 3, 2, 1));
	}

	x0 = _mm_add_epi32(x0, this->m[0]);
	x1 = _mm_add_epi32(x1, this->m[1]);
	x2 = _mm_add_epi32(x2, this->m[2]);
	x3 = _mm_add_epi32(x3, this->m[3]);
	x0 = _mm_xor_si128(x0, _mm_loadu_si128(out + 0));
	x1 = _mm_xor_si128(x1, _mm_loadu_si128(out + 1));
	x2 = _mm_xor_si128(x2, _mm_loadu_si128(out + 2));
	x3 = _mm_xor_si128(x3, _mm_loadu_si128(out + 3));
	_mm_storeu_si128(out + 0, x0);
	_mm_storeu_si128(out + 1, x1);
	_mm_storeu_si128(out + 2, x2);
	_mm_storeu_si128(out + 3, x3);

	this->m[3] = _mm_add_epi32(this->m[3], _mm_set_epi32(0, 0, 0, 1));
}

/**
 * XOR four Chacha20 keystream blocks into data, increment counter
 */
static void chacha_4block_xor(private_chapoly_drv_ssse3_t *this, void *data)
{
	__m128i x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe, xf;
	__m128i r8, r16, ctrinc, t, *out = data;
	uint32_t *m = (uint32_t*)this->m;
	u_int i;

	r8  = _mm_set_epi8(14, 13, 12, 15, 10, 9, 8, 11, 6, 5, 4, 7, 2, 1, 0, 3);
	r16 = _mm_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);
	ctrinc = _mm_set_epi32(3, 2, 1, 0);

	x0 = _mm_set1_epi32(m[ 0]);
	x1 = _mm_set1_epi32(m[ 1]);
	x2 = _mm_set1_epi32(m[ 2]);
	x3 = _mm_set1_epi32(m[ 3]);
	x4 = _mm_set1_epi32(m[ 4]);
	x5 = _mm_set1_epi32(m[ 5]);
	x6 = _mm_set1_epi32(m[ 6]);
	x7 = _mm_set1_epi32(m[ 7]);
	x8 = _mm_set1_epi32(m[ 8]);
	x9 = _mm_set1_epi32(m[ 9]);
	xa = _mm_set1_epi32(m[10]);
	xb = _mm_set1_epi32(m[11]);
	xc = _mm_set1_epi32(m[12]);
	xd = _mm_set1_epi32(m[13]);
	xe = _mm_set1_epi32(m[14]);
	xf = _mm_set1_epi32(m[15]);

	xc = _mm_add_epi32(xc, ctrinc);

	for (i = 0 ; i < CHACHA_DOUBLEROUNDS; i++)
	{
		x0 = _mm_add_epi32(x0, x4); xc = sfflxor32(xc, x0, r16);
		x1 = _mm_add_epi32(x1, x5); xd = sfflxor32(xd, x1, r16);
		x2 = _mm_add_epi32(x2, x6); xe = sfflxor32(xe, x2, r16);
		x3 = _mm_add_epi32(x3, x7); xf = sfflxor32(xf, x3, r16);

		x8 = _mm_add_epi32(x8, xc); x4 = rotlxor32(x4, x8, 12);
		x9 = _mm_add_epi32(x9, xd); x5 = rotlxor32(x5, x9, 12);
		xa = _mm_add_epi32(xa, xe); x6 = rotlxor32(x6, xa, 12);
		xb = _mm_add_epi32(xb, xf); x7 = rotlxor32(x7, xb, 12);

		x0 = _mm_add_epi32(x0, x4); xc = sfflxor32(xc, x0, r8);
		x1 = _mm_add_epi32(x1, x5); xd = sfflxor32(xd, x1, r8);
		x2 = _mm_add_epi32(x2, x6); xe = sfflxor32(xe, x2, r8);
		x3 = _mm_add_epi32(x3, x7); xf = sfflxor32(xf, x3, r8);

		x8 = _mm_add_epi32(x8, xc); x4 = rotlxor32(x4, x8, 7);
		x9 = _mm_add_epi32(x9, xd); x5 = rotlxor32(x5, x9, 7);
		xa = _mm_add_epi32(xa, xe); x6 = rotlxor32(x6, xa, 7);
		xb = _mm_add_epi32(xb, xf); x7 = rotlxor32(x7, xb, 7);

		x0 = _mm_add_epi32(x0, x5); xf = sfflxor32(xf, x0, r16);
		x1 = _mm_add_epi32(x1, x6); xc = sfflxor32(xc, x1, r16);
		x2 = _mm_add_epi32(x2, x7); xd = sfflxor32(xd, x2, r16);
		x3 = _mm_add_epi32(x3, x4); xe = sfflxor32(xe, x3, r16);

		xa = _mm_add_epi32(xa, xf); x5 = rotlxor32(x5, xa, 12);
		xb = _mm_add_epi32(xb, xc); x6 = rotlxor32(x6, xb, 12);
		x8 = _mm_add_epi32(x8, xd); x7 = rotlxor32(x7, x8, 12);
		x9 = _mm_add_epi32(x9, xe); x4 = rotlxor32(x4, x9, 12);

		x0 = _mm_add_epi32(x0, x5); xf = sfflxor32(xf, x0, r8);
		x1 = _mm_add_epi32(x1, x6); xc = sfflxor32(xc, x1, r8);
		x2 = _mm_add_epi32(x2, x7); xd = sfflxor32(xd, x2, r8);
		x3 = _mm_add_epi32(x3, x4); xe = sfflxor32(xe, x3, r8);

		xa = _mm_add_epi32(xa, xf); x5 = rotlxor32(x5, xa, 7);
		xb = _mm_add_epi32(xb, xc); x6 = rotlxor32(x6, xb, 7);
		x8 = _mm_add_epi32(x8, xd); x7 = rotlxor32(x7, x8, 7);
		x9 = _mm_add_epi32(x9, xe); x4 = rotlxor32(x4, x9, 7);
	}

	x0 = _mm_add_epi32(x0, _mm_set1_epi32(m[ 0]));
	x1 = _mm_add_epi32(x1, _mm_set1_epi32(m[ 1]));
	x2 = _mm_add_epi32(x2, _mm_set1_epi32(m[ 2]));
	x3 = _mm_add_epi32(x3, _mm_set1_epi32(m[ 3]));
	x4 = _mm_add_epi32(x4, _mm_set1_epi32(m[ 4]));
	x5 = _mm_add_epi32(x5, _mm_set1_epi32(m[ 5]));
	x6 = _mm_add_epi32(x6, _mm_set1_epi32(m[ 6]));
	x7 = _mm_add_epi32(x7, _mm_set1_epi32(m[ 7]));
	x8 = _mm_add_epi32(x8, _mm_set1_epi32(m[ 8]));
	x9 = _mm_add_epi32(x9, _mm_set1_epi32(m[ 9]));
	xa = _mm_add_epi32(xa, _mm_set1_epi32(m[10]));
	xb = _mm_add_epi32(xb, _mm_set1_epi32(m[11]));
	xc = _mm_add_epi32(xc, _mm_set1_epi32(m[12]));
	xd = _mm_add_epi32(xd, _mm_set1_epi32(m[13]));
	xe = _mm_add_epi32(xe, _mm_set1_epi32(m[14]));
	xf = _mm_add_epi32(xf, _mm_set1_epi32(m[15]));

	xc = _mm_add_epi32(xc, ctrinc);

	/* transpose state matrix by interleaving 32-, then 64-bit words */
	t = x0;	x0 = _mm_unpacklo_epi32(t, x1);
			x1 = _mm_unpackhi_epi32(t, x1);
	t = x2;	x2 = _mm_unpacklo_epi32(t, x3);
			x3 = _mm_unpackhi_epi32(t, x3);
	t = x4;	x4 = _mm_unpacklo_epi32(t, x5);
			x5 = _mm_unpackhi_epi32(t, x5);
	t = x6;	x6 = _mm_unpacklo_epi32(t, x7);
			x7 = _mm_unpackhi_epi32(t, x7);
	t = x8;	x8 = _mm_unpacklo_epi32(t, x9);
			x9 = _mm_unpackhi_epi32(t, x9);
	t = xa;	xa = _mm_unpacklo_epi32(t, xb);
			xb = _mm_unpackhi_epi32(t, xb);
	t = xc;	xc = _mm_unpacklo_epi32(t, xd);
			xd = _mm_unpackhi_epi32(t, xd);
	t = xe;	xe = _mm_unpacklo_epi32(t, xf);
			xf = _mm_unpackhi_epi32(t, xf);

	t = x0;	x0 = _mm_unpacklo_epi64(t, x2);
			x2 = _mm_unpackhi_epi64(t, x2);
	t = x1;	x1 = _mm_unpacklo_epi64(t, x3);
			x3 = _mm_unpackhi_epi64(t, x3);
	t = x4;	x4 = _mm_unpacklo_epi64(t, x6);
			x6 = _mm_unpackhi_epi64(t, x6);
	t = x5;	x5 = _mm_unpacklo_epi64(t, x7);
			x7 = _mm_unpackhi_epi64(t, x7);
	t = x8;	x8 = _mm_unpacklo_epi64(t, xa);
			xa = _mm_unpackhi_epi64(t, xa);
	t = x9;	x9 = _mm_unpacklo_epi64(t, xb);
			xb = _mm_unpackhi_epi64(t, xb);
	t = xc;	xc = _mm_unpacklo_epi64(t, xe);
			xe = _mm_unpackhi_epi64(t, xe);
	t = xd;	xd = _mm_unpacklo_epi64(t, xf);
			xf = _mm_unpackhi_epi64(t, xf);

	x0 = _mm_xor_si128(_mm_loadu_si128(out +  0), x0);
	x1 = _mm_xor_si128(_mm_loadu_si128(out +  8), x1);
	x2 = _mm_xor_si128(_mm_loadu_si128(out +  4), x2);
	x3 = _mm_xor_si128(_mm_loadu_si128(out + 12), x3);
	x4 = _mm_xor_si128(_mm_loadu_si128(out +  1), x4);
	x5 = _mm_xor_si128(_mm_loadu_si128(out +  9), x5);
	x6 = _mm_xor_si128(_mm_loadu_si128(out +  5), x6);
	x7 = _mm_xor_si128(_mm_loadu_si128(out + 13), x7);
	x8 = _mm_xor_si128(_mm_loadu_si128(out +  2), x8);
	x9 = _mm_xor_si128(_mm_loadu_si128(out + 10), x9);
	xa = _mm_xor_si128(_mm_loadu_si128(out +  6), xa);
	xb = _mm_xor_si128(_mm_loadu_si128(out + 14), xb);
	xc = _mm_xor_si128(_mm_loadu_si128(out +  3), xc);
	xd = _mm_xor_si128(_mm_loadu_si128(out + 11), xd);
	xe = _mm_xor_si128(_mm_loadu_si128(out +  7), xe);
	xf = _mm_xor_si128(_mm_loadu_si128(out + 15), xf);

	_mm_storeu_si128(out +  0, x0);
	_mm_storeu_si128(out +  8, x1);
	_mm_storeu_si128(out +  4, x2);
	_mm_storeu_si128(out + 12, x3);
	_mm_storeu_si128(out +  1, x4);
	_mm_storeu_si128(out +  9, x5);
	_mm_storeu_si128(out +  5, x6);
	_mm_storeu_si128(out + 13, x7);
	_mm_storeu_si128(out +  2, x8);
	_mm_storeu_si128(out + 10, x9);
	_mm_storeu_si128(out +  6, xa);
	_mm_storeu_si128(out + 14, xb);
	_mm_storeu_si128(out +  3, xc);
	_mm_storeu_si128(out + 11, xd);
	_mm_storeu_si128(out +  7, xe);
	_mm_storeu_si128(out + 15, xf);

	this->m[3] = _mm_add_epi32(this->m[3], _mm_set_epi32(0, 0, 0, 4));
}

METHOD(chapoly_drv_t, set_key, bool,
	private_chapoly_drv_ssse3_t *this, u_char *constant, u_char *key,
	u_char *salt)
{
	this->m[0] = _mm_loadu_si128((__m128i*)constant);
	this->m[1] = _mm_loadu_si128((__m128i*)key + 0);
	this->m[2] = _mm_loadu_si128((__m128i*)key + 1);
	this->m[3] = _mm_set_epi32(0, 0, ru32(salt), 0);

	return TRUE;
}

/**
 * r[127:64] = h[95:64] * a, r[63:0] = h[31:0] * b
 */
static inline __m128i mul2(__m128i h, uint32_t a, uint32_t b)
{
	return _mm_mul_epu32(h, _mm_set_epi32(0, a, 0, b));
}

/**
 * c = a[127:64] + a[63:0] + b[127:64] + b[63:0]
 * z = x[127:64] + x[63:0] + y[127:64] + y[63:0]
 */
static inline void sum2(__m128i a, __m128i b, __m128i x, __m128i y,
						uint64_t *c, uint64_t *z)
{
	__m128i r, s;

	a = _mm_add_epi64(a, b);
	x = _mm_add_epi64(x, y);
	r = _mm_unpacklo_epi64(x, a);
	s = _mm_unpackhi_epi64(x, a);
	r = _mm_add_epi64(r, s);

	_mm_storel_epi64((__m128i*)z, r);
	_mm_storel_epi64((__m128i*)c, _mm_srli_si128(r, 8));
}

/**
 * r = a[127:64] + b[127:64] + c[127:64] + d[127:64] + e[127:64]
 *   + a[63:0]   + b[63:0]   + c[63:0]   + d[63:0]   + e[63:0]
 */
static inline uint64_t sum5(__m128i a, __m128i b, __m128i c,
							 __m128i d, __m128i e)
{
	uint64_t r;

	a = _mm_add_epi64(a, b);
	c = _mm_add_epi64(c, d);
	a = _mm_add_epi64(a, e);
	a = _mm_add_epi64(a, c);

	a = _mm_add_epi64(a, _mm_srli_si128(a, 8));
	_mm_storel_epi64((__m128i*)&r, a);

	return r;
}

/**
 * Make second Poly1305 key u = r^2
 */
static void make_u(private_chapoly_drv_ssse3_t *this)
{
	__m128i r01, r23, r44, x0, x1, y0, y1, z0;
	uint32_t r0, r1, r2, r3, r4;
	uint32_t u0, u1, u2, u3, u4;
	uint32_t s1, s2, s3, s4;
	uint64_t d0, d1, d2, d3, d4;

	r0 = this->r[0];
	r1 = this->r[1];
	r2 = this->r[2];
	r3 = this->r[3];
	r4 = this->r[4];

	s1 = r1 * 5;
	s2 = r2 * 5;
	s3 = r3 * 5;
	s4 = r4 * 5;

	r01 = _mm_set_epi32(0, r0, 0, r1);
	r23 = _mm_set_epi32(0, r2, 0, r3);
	r44 = _mm_set_epi32(0, r4, 0, r4);

	/* u = r^2 */
	x0 = mul2(r01, r0, s4);
	x1 = mul2(r01, r1, r0);
	y0 = mul2(r23, s3, s2);
	y1 = mul2(r23, s4, s3);
	z0 = mul2(r44, s1, s2);
	y0 = _mm_add_epi64(y0, _mm_srli_si128(z0, 8));
	y1 = _mm_add_epi64(y1, _mm_slli_si128(z0, 8));
	sum2(x0, y0, x1, y1, &d0, &d1);

	x0 = mul2(r01, r2, r1);
	x1 = mul2(r01, r3, r2);
	y0 = mul2(r23, r0, s4);
	y1 = mul2(r23, r1, r0);
	z0 = mul2(r44, s3, s4);
	y0 = _mm_add_epi64(y0, _mm_srli_si128(z0, 8));
	y1 = _mm_add_epi64(y1, _mm_slli_si128(z0, 8));
	sum2(x0, y0, x1, y1, &d2, &d3);

	x0 = mul2(r01, r4, r3);
	y0 = mul2(r23, r2, r1);
	z0 = mul2(r44, r0, 0);
	y0 = _mm_add_epi64(y0, z0);
	x0 = _mm_add_epi64(x0, y0);
	x0 = _mm_add_epi64(x0, _mm_srli_si128(x0, 8));
	_mm_storel_epi64((__m128i*)&d4, x0);

	/* (partial) r %= p */
	d1 += sr(d0, 26);     u0 = and(d0, 0x3ffffff);
	d2 += sr(d1, 26);     u1 = and(d1, 0x3ffffff);
	d3 += sr(d2, 26);     u2 = and(d2, 0x3ffffff);
	d4 += sr(d3, 26);     u3 = and(d3, 0x3ffffff);
	u0 += sr(d4, 26) * 5; u4 = and(d4, 0x3ffffff);
	u1 += u0 >> 26;       u0 &= 0x3ffffff;

	this->u[0] = u0;
	this->u[1] = u1;
	this->u[2] = u2;
	this->u[3] = u3;
	this->u[4] = u4;
}

METHOD(chapoly_drv_t, init, bool,
	private_chapoly_drv_ssse3_t *this, u_char *iv)
{
	u_char key[CHACHA_BLOCK_SIZE];

	this->m[3] = _mm_or_si128(
					_mm_set_epi32(ru32(iv + 4), ru32(iv + 0), 0, 0),
					_mm_and_si128(this->m[3], _mm_set_epi32(0, 0, ~0, 0)));

	memset(key, 0, CHACHA_BLOCK_SIZE);
	chacha_block_xor(this, key);

	/* r &= 0xffffffc0ffffffc0ffffffc0fffffff */
	this->r[0] = (ru32(key +  0) >> 0) & 0x3ffffff;
	this->r[1] = (ru32(key +  3) >> 2) & 0x3ffff03;
	this->r[2] = (ru32(key +  6) >> 4) & 0x3ffc0ff;
	this->r[3] = (ru32(key +  9) >> 6) & 0x3f03fff;
	this->r[4] = (ru32(key + 12) >> 8) & 0x00fffff;

	make_u(this);

	/* h = 0 */
	memwipe(this->h, sizeof(this->h));

	this->s[0] = ru32(key + 16);
	this->s[1] = ru32(key + 20);
	this->s[2] = ru32(key + 24);
	this->s[3] = ru32(key + 28);

	return TRUE;
}

/**
 * Update Poly1305 for a multiple of two blocks
 */
static void poly2(private_chapoly_drv_ssse3_t *this, u_char *data, u_int dblks)
{
	uint32_t r0, r1, r2, r3, r4, u0, u1, u2, u3, u4;
	uint32_t s1, s2, s3, s4, v1, v2, v3, v4;
	__m128i hc0, hc1, hc2, hc3, hc4;
	uint32_t h0, h1, h2, h3, h4;
	uint32_t c0, c1, c2, c3, c4;
	uint64_t d0, d1, d2, d3, d4;
	u_int i;

	r0 = this->r[0];
	r1 = this->r[1];
	r2 = this->r[2];
	r3 = this->r[3];
	r4 = this->r[4];

	s1 = r1 * 5;
	s2 = r2 * 5;
	s3 = r3 * 5;
	s4 = r4 * 5;

	u0 = this->u[0];
	u1 = this->u[1];
	u2 = this->u[2];
	u3 = this->u[3];
	u4 = this->u[4];

	v1 = u1 * 5;
	v2 = u2 * 5;
	v3 = u3 * 5;
	v4 = u4 * 5;

	h0 = this->h[0];
	h1 = this->h[1];
	h2 = this->h[2];
	h3 = this->h[3];
	h4 = this->h[4];

	/* h = (h + c1) * r^2 + c2 * r */
	for (i = 0; i < dblks; i++)
	{
		/* h += m[i] */
		h0 += (ru32(data +  0) >> 0) & 0x3ffffff;
		h1 += (ru32(data +  3) >> 2) & 0x3ffffff;
		h2 += (ru32(data +  6) >> 4) & 0x3ffffff;
		h3 += (ru32(data +  9) >> 6) & 0x3ffffff;
		h4 += (ru32(data + 12) >> 8) | (1 << 24);
		data += POLY_BLOCK_SIZE;

		/* c = m[i + 1] */
		c0 = (ru32(data +  0) >> 0) & 0x3ffffff;
		c1 = (ru32(data +  3) >> 2) & 0x3ffffff;
		c2 = (ru32(data +  6) >> 4) & 0x3ffffff;
		c3 = (ru32(data +  9) >> 6) & 0x3ffffff;
		c4 = (ru32(data + 12) >> 8) | (1 << 24);
		data += POLY_BLOCK_SIZE;

		hc0 = _mm_set_epi32(0, h0, 0, c0);
		hc1 = _mm_set_epi32(0, h1, 0, c1);
		hc2 = _mm_set_epi32(0, h2, 0, c2);
		hc3 = _mm_set_epi32(0, h3, 0, c3);
		hc4 = _mm_set_epi32(0, h4, 0, c4);

		/* h = h * r^2 + c * r */
		d0 = sum5(mul2(hc0, u0, r0),
				  mul2(hc1, v4, s4),
				  mul2(hc2, v3, s3),
				  mul2(hc3, v2, s2),
				  mul2(hc4, v1, s1));
		d1 = sum5(mul2(hc0, u1, r1),
				  mul2(hc1, u0, r0),
				  mul2(hc2, v4, s4),
				  mul2(hc3, v3, s3),
				  mul2(hc4, v2, s2));
		d2 = sum5(mul2(hc0, u2, r2),
				  mul2(hc1, u1, r1),
				  mul2(hc2, u0, r0),
				  mul2(hc3, v4, s4),
				  mul2(hc4, v3, s3));
		d3 = sum5(mul2(hc0, u3, r3),
				  mul2(hc1, u2, r2),
				  mul2(hc2, u1, r1),
				  mul2(hc3, u0, r0),
				  mul2(hc4, v4, s4));
		d4 = sum5(mul2(hc0, u4, r4),
				  mul2(hc1, u3, r3),
				  mul2(hc2, u2, r2),
				  mul2(hc3, u1, r1),
				  mul2(hc4, u0, r0));

		/* (partial) h %= p */
		d1 += sr(d0, 26);     h0 = and(d0, 0x3ffffff);
		d2 += sr(d1, 26);     h1 = and(d1, 0x3ffffff);
		d3 += sr(d2, 26);     h2 = and(d2, 0x3ffffff);
		d4 += sr(d3, 26);     h3 = and(d3, 0x3ffffff);
		h0 += sr(d4, 26) * 5; h4 = and(d4, 0x3ffffff);
		h1 += h0 >> 26;       h0 = h0 & 0x3ffffff;
	}

	this->h[0] = h0;
	this->h[1] = h1;
	this->h[2] = h2;
	this->h[3] = h3;
	this->h[4] = h4;
}

/**
 * Update Poly1305 for a single block
 */
static void poly1(private_chapoly_drv_ssse3_t *this, u_char *data)
{
	uint32_t r0, r1, r2, r3, r4;
	uint32_t s1, s2, s3, s4;
	uint32_t h0, h1, h2, h3, h4;
	uint64_t d0, d1, d2, d3, d4;
	__m128i h01, h23, h44;
	__m128i x0, x1, y0, y1, z0;
	uint32_t t0, t1;

	r0 = this->r[0];
	r1 = this->r[1];
	r2 = this->r[2];
	r3 = this->r[3];
	r4 = this->r[4];

	s1 = r1 * 5;
	s2 = r2 * 5;
	s3 = r3 * 5;
	s4 = r4 * 5;

	h0 = this->h[0];
	h1 = this->h[1];
	h2 = this->h[2];
	h3 = this->h[3];
	h4 = this->h[4];

	h01 = _mm_set_epi32(0, h0, 0, h1);
	h23 = _mm_set_epi32(0, h2, 0, h3);
	h44 = _mm_set_epi32(0, h4, 0, h4);

	/* h += m[i] */
	t0  = (ru32(data +  0) >> 0) & 0x3ffffff;
	t1  = (ru32(data +  3) >> 2) & 0x3ffffff;
	h01 = _mm_add_epi32(h01, _mm_set_epi32(0, t0, 0, t1));
	t0  = (ru32(data +  6) >> 4) & 0x3ffffff;
	t1  = (ru32(data +  9) >> 6) & 0x3ffffff;
	h23 = _mm_add_epi32(h23, _mm_set_epi32(0, t0, 0, t1));
	t0  = (ru32(data + 12) >> 8) | (1 << 24);
	h44 = _mm_add_epi32(h44, _mm_set_epi32(0, t0, 0, t0));

	/* h *= r */
	x0 = mul2(h01, r0, s4);
	x1 = mul2(h01, r1, r0);
	y0 = mul2(h23, s3, s2);
	y1 = mul2(h23, s4, s3);
	z0 = mul2(h44, s1, s2);
	y0 = _mm_add_epi64(y0, _mm_srli_si128(z0, 8));
	y1 = _mm_add_epi64(y1, _mm_slli_si128(z0, 8));
	sum2(x0, y0, x1, y1, &d0, &d1);

	x0 = mul2(h01, r2, r1);
	x1 = mul2(h01, r3, r2);
	y0 = mul2(h23, r0, s4);
	y1 = mul2(h23, r1, r0);
	z0 = mul2(h44, s3, s4);
	y0 = _mm_add_epi64(y0, _mm_srli_si128(z0, 8));
	y1 = _mm_add_epi64(y1, _mm_slli_si128(z0, 8));
	sum2(x0, y0, x1, y1, &d2, &d3);

	x0 = mul2(h01, r4, r3);
	y0 = mul2(h23, r2, r1);
	z0 = mul2(h44, r0, 0);
	y0 = _mm_add_epi64(y0, z0);
	x0 = _mm_add_epi64(x0, y0);
	x0 = _mm_add_epi64(x0, _mm_srli_si128(x0, 8));
	_mm_storel_epi64((__m128i*)&d4, x0);

	/* (partial) h %= p */
	d1 += sr(d0, 26);     h0 = and(d0, 0x3ffffff);
	d2 += sr(d1, 26);     h1 = and(d1, 0x3ffffff);
	d3 += sr(d2, 26);     h2 = and(d2, 0x3ffffff);
	d4 += sr(d3, 26);     h3 = and(d3, 0x3ffffff);
	h0 += sr(d4, 26) * 5; h4 = and(d4, 0x3ffffff);
	h1 += h0 >> 26;       h0 = h0 & 0x3ffffff;

	this->h[0] = h0;
	this->h[1] = h1;
	this->h[2] = h2;
	this->h[3] = h3;
	this->h[4] = h4;
}

METHOD(chapoly_drv_t, poly, bool,
	private_chapoly_drv_ssse3_t *this, u_char *data, u_int blocks)
{
	poly2(this, data, blocks / 2);
	if (blocks-- % 2)
	{
		poly1(this, data + POLY_BLOCK_SIZE * blocks);
	}
	return TRUE;
}

METHOD(chapoly_drv_t, chacha, bool,
	private_chapoly_drv_ssse3_t *this, u_char *stream)
{
	memset(stream, 0, CHACHA_BLOCK_SIZE);
	chacha_block_xor(this, stream);

	return TRUE;
}

METHOD(chapoly_drv_t, encrypt, bool,
	private_chapoly_drv_ssse3_t *this, u_char *data, u_int blocks)
{
	while (blocks >= 4)
	{
		chacha_4block_xor(this, data);
		poly2(this, data, 8);
		data += CHACHA_BLOCK_SIZE * 4;
		blocks -= 4;
	}
	while (blocks--)
	{
		chacha_block_xor(this, data);
		poly2(this, data, 2);
		data += CHACHA_BLOCK_SIZE;
	}
	return TRUE;
}

METHOD(chapoly_drv_t, decrypt, bool,
	private_chapoly_drv_ssse3_t *this, u_char *data, u_int blocks)
{
	while (blocks >= 4)
	{
		poly2(this, data, 8);
		chacha_4block_xor(this, data);
		data += CHACHA_BLOCK_SIZE * 4;
		blocks -= 4;
	}
	while (blocks--)
	{
		poly2(this, data, 2);
		chacha_block_xor(this, data);
		data += CHACHA_BLOCK_SIZE;
	}
	return TRUE;
}

METHOD(chapoly_drv_t, finish, bool,
	private_chapoly_drv_ssse3_t *this, u_char *mac)
{
	uint32_t h0, h1, h2, h3, h4;
	uint32_t g0, g1, g2, g3, g4;
	uint32_t mask;
	uint64_t f = 0;

	/* fully carry h */
	h0 = this->h[0];
	h1 = this->h[1];
	h2 = this->h[2];
	h3 = this->h[3];
	h4 = this->h[4];

	h2 += (h1 >> 26);     h1 = h1 & 0x3ffffff;
	h3 += (h2 >> 26);     h2 = h2 & 0x3ffffff;
	h4 += (h3 >> 26);     h3 = h3 & 0x3ffffff;
	h0 += (h4 >> 26) * 5; h4 = h4 & 0x3ffffff;
	h1 += (h0 >> 26);     h0 = h0 & 0x3ffffff;

	/* compute h + -p */
	g0 = h0 + 5;
	g1 = h1 + (g0 >> 26);             g0 &= 0x3ffffff;
	g2 = h2 + (g1 >> 26);             g1 &= 0x3ffffff;
	g3 = h3 + (g2 >> 26);             g2 &= 0x3ffffff;
	g4 = h4 + (g3 >> 26) - (1 << 26); g3 &= 0x3ffffff;

	/* select h if h < p, or h + -p if h >= p */
	mask = (g4 >> ((sizeof(uint32_t) * 8) - 1)) - 1;
	g0 &= mask;
	g1 &= mask;
	g2 &= mask;
	g3 &= mask;
	g4 &= mask;
	mask = ~mask;
	h0 = (h0 & mask) | g0;
	h1 = (h1 & mask) | g1;
	h2 = (h2 & mask) | g2;
	h3 = (h3 & mask) | g3;
	h4 = (h4 & mask) | g4;

	/* h = h % (2^128) */
	h0 = (h0 >>  0) | (h1 << 26);
	h1 = (h1 >>  6) | (h2 << 20);
	h2 = (h2 >> 12) | (h3 << 14);
	h3 = (h3 >> 18) | (h4 <<  8);

	/* mac = (h + s) % (2^128) */
	f = (f >> 32) + h0 + this->s[0]; wu32(mac +  0, f);
	f = (f >> 32) + h1 + this->s[1]; wu32(mac +  4, f);
	f = (f >> 32) + h2 + this->s[2]; wu32(mac +  8, f);
	f = (f >> 32) + h3 + this->s[3]; wu32(mac + 12, f);

	return TRUE;
}

METHOD(chapoly_drv_t, destroy, void,
	private_chapoly_drv_ssse3_t *this)
{
	memwipe(this->m, sizeof(this->m));
	memwipe(this->h, sizeof(this->h));
	memwipe(this->r, sizeof(this->r));
	memwipe(this->u, sizeof(this->u));
	memwipe(this->s, sizeof(this->s));
	free_align(this);
}

/**
 * See header
 */
chapoly_drv_t *chapoly_drv_ssse3_create()
{
	private_chapoly_drv_ssse3_t *this;

	if (!cpu_feature_available(CPU_FEATURE_SSSE3))
	{
		return FALSE;
	}

	INIT_ALIGN(this, sizeof(__m128i),
		.public = {
			.set_key = _set_key,
			.init = _init,
			.poly = _poly,
			.chacha = _chacha,
			.encrypt = _encrypt,
			.decrypt = _decrypt,
			.finish = _finish,
			.destroy = _destroy,
		},
	);

	return &this->public;
}

#else /* !__SSSE3__ */

chapoly_drv_t *chapoly_drv_ssse3_create()
{
	return NULL;
}

#endif /* !__SSSE3__ */
