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

#include "chapoly_drv_portable.h"

#define CHACHA_DOUBLEROUNDS 10
/* index of some state fields */
#define CHACHA_BLOCKCOUNT 12
#define CHACHA_NONCE1 13
#define CHACHA_NONCE2 14
#define CHACHA_NONCE3 15

typedef struct private_chapoly_drv_portable_t private_chapoly_drv_portable_t;

/**
 * Private data of an chapoly_drv_portable_t object.
 */
struct private_chapoly_drv_portable_t {

	/**
	 * Public chapoly_drv_portable_t interface.
	 */
	chapoly_drv_t public;

	/**
	 * ChaCha20 state matrix
	 */
	uint32_t m[16];

	/**
	 * Poly1305 update key
	 */
	uint32_t r[5];

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
 * XOR a 32-bit integer into an unaligned destination
 */
static inline void xor32u(void *p, uint32_t x)
{
	uint32_t y;

	memcpy(&y, p, sizeof(y));
	y ^= x;
	memcpy(p, &y, sizeof(y));
}

/**
 * Multiply two 64-bit words
 */
static inline uint64_t mlt(uint64_t a, uint64_t b)
{
	return a * b;
}

/**
 * Shift a 64-bit unsigned integer v right by n bits, clamp to 32 bit
*/
static inline uint32_t sr(uint64_t v, u_char n)
{
	return v >> n;
}

/**
 * Circular left shift by n bits
 */
static inline uint32_t rotl32(uint32_t v, u_char n)
{
	return (v << n) | (v >> (sizeof(v) * 8 - n));
}

/**
 * AND two values, using a native integer size >= sizeof(uint32_t)
 */
static inline u_long and(u_long v, u_long mask)
{
	return v & mask;
}

/**
 * XOR a Chacha20 keystream block into data, increment counter
 */
static void chacha_block_xor(private_chapoly_drv_portable_t *this, void *data)
{
	uint32_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe, xf;
	uint32_t *out = data;
	u_int i;

	x0 = this->m[ 0];
	x1 = this->m[ 1];
	x2 = this->m[ 2];
	x3 = this->m[ 3];
	x4 = this->m[ 4];
	x5 = this->m[ 5];
	x6 = this->m[ 6];
	x7 = this->m[ 7];
	x8 = this->m[ 8];
	x9 = this->m[ 9];
	xa = this->m[10];
	xb = this->m[11];
	xc = this->m[12];
	xd = this->m[13];
	xe = this->m[14];
	xf = this->m[15];

	for (i = 0; i < CHACHA_DOUBLEROUNDS; i++)
	{
		x0 += x4; xc = rotl32(xc ^ x0, 16);
		x1 += x5; xd = rotl32(xd ^ x1, 16);
		x2 += x6; xe = rotl32(xe ^ x2, 16);
		x3 += x7; xf = rotl32(xf ^ x3, 16);

		x8 += xc; x4 = rotl32(x4 ^ x8, 12);
		x9 += xd; x5 = rotl32(x5 ^ x9, 12);
		xa += xe; x6 = rotl32(x6 ^ xa, 12);
		xb += xf; x7 = rotl32(x7 ^ xb, 12);

		x0 += x4; xc = rotl32(xc ^ x0, 8);
		x1 += x5; xd = rotl32(xd ^ x1, 8);
		x2 += x6; xe = rotl32(xe ^ x2, 8);
		x3 += x7; xf = rotl32(xf ^ x3, 8);

		x8 += xc; x4 = rotl32(x4 ^ x8, 7);
		x9 += xd; x5 = rotl32(x5 ^ x9, 7);
		xa += xe; x6 = rotl32(x6 ^ xa, 7);
		xb += xf; x7 = rotl32(x7 ^ xb, 7);

		x0 += x5; xf = rotl32(xf ^ x0, 16);
		x1 += x6; xc = rotl32(xc ^ x1, 16);
		x2 += x7; xd = rotl32(xd ^ x2, 16);
		x3 += x4; xe = rotl32(xe ^ x3, 16);

		xa += xf; x5 = rotl32(x5 ^ xa, 12);
		xb += xc; x6 = rotl32(x6 ^ xb, 12);
		x8 += xd; x7 = rotl32(x7 ^ x8, 12);
		x9 += xe; x4 = rotl32(x4 ^ x9, 12);

		x0 += x5; xf = rotl32(xf ^ x0, 8);
		x1 += x6; xc = rotl32(xc ^ x1, 8);
		x2 += x7; xd = rotl32(xd ^ x2, 8);
		x3 += x4; xe = rotl32(xe ^ x3, 8);

		xa += xf; x5 = rotl32(x5 ^ xa, 7);
		xb += xc; x6 = rotl32(x6 ^ xb, 7);
		x8 += xd; x7 = rotl32(x7 ^ x8, 7);
		x9 += xe; x4 = rotl32(x4 ^ x9, 7);
	}

	xor32u(out +  0, le32toh(x0 + this->m[ 0]));
	xor32u(out +  1, le32toh(x1 + this->m[ 1]));
	xor32u(out +  2, le32toh(x2 + this->m[ 2]));
	xor32u(out +  3, le32toh(x3 + this->m[ 3]));
	xor32u(out +  4, le32toh(x4 + this->m[ 4]));
	xor32u(out +  5, le32toh(x5 + this->m[ 5]));
	xor32u(out +  6, le32toh(x6 + this->m[ 6]));
	xor32u(out +  7, le32toh(x7 + this->m[ 7]));
	xor32u(out +  8, le32toh(x8 + this->m[ 8]));
	xor32u(out +  9, le32toh(x9 + this->m[ 9]));
	xor32u(out + 10, le32toh(xa + this->m[10]));
	xor32u(out + 11, le32toh(xb + this->m[11]));
	xor32u(out + 12, le32toh(xc + this->m[12]));
	xor32u(out + 13, le32toh(xd + this->m[13]));
	xor32u(out + 14, le32toh(xe + this->m[14]));
	xor32u(out + 15, le32toh(xf + this->m[15]));

	this->m[CHACHA_BLOCKCOUNT]++;
}

METHOD(chapoly_drv_t, set_key, bool,
	private_chapoly_drv_portable_t *this, u_char *constant, u_char *key,
	u_char *salt)
{
	this->m[ 0] = uletoh32(constant +  0);
	this->m[ 1] = uletoh32(constant +  4);
	this->m[ 2] = uletoh32(constant +  8);
	this->m[ 3] = uletoh32(constant + 12);

	this->m[ 4] = uletoh32(key +  0);
	this->m[ 5] = uletoh32(key +  4);
	this->m[ 6] = uletoh32(key +  8);
	this->m[ 7] = uletoh32(key + 12);
	this->m[ 8] = uletoh32(key + 16);
	this->m[ 9] = uletoh32(key + 20);
	this->m[10] = uletoh32(key + 24);
	this->m[11] = uletoh32(key + 28);

	this->m[CHACHA_NONCE1] = uletoh32(salt);

	return TRUE;
}

METHOD(chapoly_drv_t, init, bool,
	private_chapoly_drv_portable_t *this, u_char *iv)
{
	u_char key[CHACHA_BLOCK_SIZE];

	this->m[CHACHA_BLOCKCOUNT] = 0;
	this->m[CHACHA_NONCE2] = uletoh32(iv + 0);
	this->m[CHACHA_NONCE3] = uletoh32(iv + 4);

	memset(key, 0, CHACHA_BLOCK_SIZE);
	chacha_block_xor(this, key);

	/* r &= 0xffffffc0ffffffc0ffffffc0fffffff */
	this->r[0] = (uletoh32(key +  0) >> 0) & 0x3ffffff;
	this->r[1] = (uletoh32(key +  3) >> 2) & 0x3ffff03;
	this->r[2] = (uletoh32(key +  6) >> 4) & 0x3ffc0ff;
	this->r[3] = (uletoh32(key +  9) >> 6) & 0x3f03fff;
	this->r[4] = (uletoh32(key + 12) >> 8) & 0x00fffff;

	/* h = 0 */
	memwipe(this->h, sizeof(this->h));

	this->s[0] = uletoh32(key + 16);
	this->s[1] = uletoh32(key + 20);
	this->s[2] = uletoh32(key + 24);
	this->s[3] = uletoh32(key + 28);

	return TRUE;
}

METHOD(chapoly_drv_t, poly, bool,
	private_chapoly_drv_portable_t *this, u_char *data, u_int blocks)
{
	uint32_t r0, r1, r2, r3, r4;
	uint32_t s1, s2, s3, s4;
	uint32_t h0, h1, h2, h3, h4;
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

	h0 = this->h[0];
	h1 = this->h[1];
	h2 = this->h[2];
	h3 = this->h[3];
	h4 = this->h[4];

	for (i = 0; i < blocks; i++)
	{
		/* h += m[i] */
		h0 += (uletoh32(data +  0) >> 0) & 0x3ffffff;
		h1 += (uletoh32(data +  3) >> 2) & 0x3ffffff;
		h2 += (uletoh32(data +  6) >> 4) & 0x3ffffff;
		h3 += (uletoh32(data +  9) >> 6) & 0x3ffffff;
		h4 += (uletoh32(data + 12) >> 8) | (1 << 24);

		/* h *= r */
		d0 = mlt(h0, r0) + mlt(h1, s4) + mlt(h2, s3) + mlt(h3, s2) + mlt(h4, s1);
		d1 = mlt(h0, r1) + mlt(h1, r0) + mlt(h2, s4) + mlt(h3, s3) + mlt(h4, s2);
		d2 = mlt(h0, r2) + mlt(h1, r1) + mlt(h2, r0) + mlt(h3, s4) + mlt(h4, s3);
		d3 = mlt(h0, r3) + mlt(h1, r2) + mlt(h2, r1) + mlt(h3, r0) + mlt(h4, s4);
		d4 = mlt(h0, r4) + mlt(h1, r3) + mlt(h2, r2) + mlt(h3, r1) + mlt(h4, r0);

		/* (partial) h %= p */
		d1 += sr(d0, 26);     h0 = and(d0, 0x3ffffff);
		d2 += sr(d1, 26);     h1 = and(d1, 0x3ffffff);
		d3 += sr(d2, 26);     h2 = and(d2, 0x3ffffff);
		d4 += sr(d3, 26);     h3 = and(d3, 0x3ffffff);
		h0 += sr(d4, 26) * 5; h4 = and(d4, 0x3ffffff);
		h1 += h0 >> 26;       h0 = h0 & 0x3ffffff;

		data += POLY_BLOCK_SIZE;
	}

	this->h[0] = h0;
	this->h[1] = h1;
	this->h[2] = h2;
	this->h[3] = h3;
	this->h[4] = h4;

	return TRUE;
}

METHOD(chapoly_drv_t, chacha, bool,
	private_chapoly_drv_portable_t *this, u_char *stream)
{
	memset(stream, 0, CHACHA_BLOCK_SIZE);
	chacha_block_xor(this, stream);

	return TRUE;
}

METHOD(chapoly_drv_t, encrypt, bool,
	private_chapoly_drv_portable_t *this, u_char *data, u_int blocks)
{
	u_int i;

	for (i = 0; i < blocks; i++)
	{
		chacha_block_xor(this, data);
		poly(this, data, 4);
		data += CHACHA_BLOCK_SIZE;
	}
	return TRUE;
}

METHOD(chapoly_drv_t, decrypt, bool,
	private_chapoly_drv_portable_t *this, u_char *data, u_int blocks)
{
	u_int i;

	for (i = 0; i < blocks; i++)
	{
		poly(this, data, 4);
		chacha_block_xor(this, data);
		data += CHACHA_BLOCK_SIZE;
	}
	return TRUE;
}

METHOD(chapoly_drv_t, finish, bool,
	private_chapoly_drv_portable_t *this, u_char *mac)
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
	f = (f >> 32) + h0 + this->s[0]; htoule32(mac +  0, f);
	f = (f >> 32) + h1 + this->s[1]; htoule32(mac +  4, f);
	f = (f >> 32) + h2 + this->s[2]; htoule32(mac +  8, f);
	f = (f >> 32) + h3 + this->s[3]; htoule32(mac + 12, f);

	return TRUE;
}

METHOD(chapoly_drv_t, destroy, void,
	private_chapoly_drv_portable_t *this)
{
	memwipe(this->m, sizeof(this->m));
	memwipe(this->h, sizeof(this->h));
	memwipe(this->r, sizeof(this->r));
	memwipe(this->s, sizeof(this->s));
	free(this);
}

/**
 * See header
 */
chapoly_drv_t *chapoly_drv_portable_create()
{
	private_chapoly_drv_portable_t *this;

	INIT(this,
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
