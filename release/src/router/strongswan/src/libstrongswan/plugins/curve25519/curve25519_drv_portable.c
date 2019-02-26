/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
 *
 * Based on public domain code by Andrew Moon (curve22519-donna).
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

#include "curve25519_drv_portable.h"

typedef struct private_curve25519_drv_t private_curve25519_drv_t;

/**
 * Private data of an curve25519_drv_portable_t object.
 */
struct private_curve25519_drv_t {

	/**
	 * Public curve25519_drv_t interface.
	 */
	curve25519_drv_t public;

	/**
	 * Private key
	 */
	u_char key[CURVE25519_KEY_SIZE];
};

METHOD(curve25519_drv_t, set_key, bool,
	private_curve25519_drv_t *this, u_char *key)
{
	memcpy(this->key, key, sizeof(this->key));

	this->key[0] &= 0xf8;
	this->key[31] &= 0x7f;
	this->key[31] |= 0x40;
	return TRUE;
}

/**
 * OR a 32-bit integer to an unaligned little-endian
 */
static inline void horule32(void *p, uint32_t x)
{
	uint32_t r;

	memcpy(&r, p, sizeof(r));
	r |= htole32(x);
	memcpy(p, &r, sizeof(r));
}

/**
 * Reduce a 32-bit integer to 26 bits
 */
static inline uint32_t rdc26(uint32_t v)
{
	return v & ((1 << 26) - 1);
}

/**
 * Reduce a 32-bit integer to 25 bits
 */
static inline uint32_t rdc25(uint32_t v)
{
	return v & ((1 << 25) - 1);
}

/**
 * Shift right a 64-bit integer by 26 bits
 */
static inline uint32_t sr26(uint64_t v)
{
	return v >> 26;
}

/**
 * Shift right a 64-bit integer by 25 bits
 */
static inline uint32_t sr25(uint64_t v)
{
	return v >> 25;
}

/**
 * Multiply a 64-bit integer with a 32-bit integer
 */
static inline uint64_t mul64(uint64_t a, uint32_t b)
{
	return a * b;
}

/**
 * out = a + b
 */
static inline void add(uint32_t out[10], uint32_t a[10], uint32_t b[10])
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
	out[3] = a[3] + b[3];
	out[4] = a[4] + b[4];
	out[5] = a[5] + b[5];
	out[6] = a[6] + b[6];
	out[7] = a[7] + b[7];
	out[8] = a[8] + b[8];
	out[9] = a[9] + b[9];
}

/**
 * out = a - b
 */
static inline void sub(uint32_t out[10], uint32_t a[10], uint32_t b[10])
{
	uint32_t x;

	x = 0x7ffffda + a[0] - b[0];           out[0] = rdc26(x);
	x = 0x3fffffe + a[1] - b[1] + sr26(x); out[1] = rdc25(x);
	x = 0x7fffffe + a[2] - b[2] + sr25(x); out[2] = rdc26(x);
	x = 0x3fffffe + a[3] - b[3] + sr26(x); out[3] = rdc25(x);
	x = 0x7fffffe + a[4] - b[4] + sr25(x); out[4] = rdc26(x);
	x = 0x3fffffe + a[5] - b[5] + sr26(x); out[5] = rdc25(x);
	x = 0x7fffffe + a[6] - b[6] + sr25(x); out[6] = rdc26(x);
	x = 0x3fffffe + a[7] - b[7] + sr26(x); out[7] = rdc25(x);
	x = 0x7fffffe + a[8] - b[8] + sr25(x); out[8] = rdc26(x);
	x = 0x3fffffe + a[9] - b[9] + sr26(x); out[9] = rdc25(x);
	                    out[0] += sr25(x) * 19;
}

/**
 * out = in * scalar
 */
static void scalar_product(uint32_t out[10], uint32_t in[10], uint32_t scalar)
{
	uint64_t x;

	x = mul64(in[0], scalar);           out[0] = rdc26(x);
	x = mul64(in[1], scalar) + sr26(x); out[1] = rdc25(x);
	x = mul64(in[2], scalar) + sr25(x); out[2] = rdc26(x);
	x = mul64(in[3], scalar) + sr26(x); out[3] = rdc25(x);
	x = mul64(in[4], scalar) + sr25(x); out[4] = rdc26(x);
	x = mul64(in[5], scalar) + sr26(x); out[5] = rdc25(x);
	x = mul64(in[6], scalar) + sr25(x); out[6] = rdc26(x);
	x = mul64(in[7], scalar) + sr26(x); out[7] = rdc25(x);
	x = mul64(in[8], scalar) + sr25(x); out[8] = rdc26(x);
	x = mul64(in[9], scalar) + sr26(x); out[9] = rdc25(x);
	                 out[0] += sr25(x) * 19;
}

/**
 * out = a * b
 */
static inline void mul(uint32_t out[10], uint32_t a[10], uint32_t b[10])
{
	uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
	uint32_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9;
	uint64_t m0, m1, m2, m3, m4, m5, m6, m7, m8, m9;

	r0 = b[0];
	r1 = b[1];
	r2 = b[2];
	r3 = b[3];
	r4 = b[4];
	r5 = b[5];
	r6 = b[6];
	r7 = b[7];
	r8 = b[8];
	r9 = b[9];

	s0 = a[0];
	s1 = a[1];
	s2 = a[2];
	s3 = a[3];
	s4 = a[4];
	s5 = a[5];
	s6 = a[6];
	s7 = a[7];
	s8 = a[8];
	s9 = a[9];

	m1 = mul64(r0, s1) + mul64(r1, s0);
	m3 = mul64(r0, s3) + mul64(r1, s2) + mul64(r2, s1) + mul64(r3, s0);
	m5 = mul64(r0, s5) + mul64(r1, s4) + mul64(r2, s3) + mul64(r3, s2)
	   + mul64(r4, s1) + mul64(r5, s0);
	m7 = mul64(r0, s7) + mul64(r1, s6) + mul64(r2, s5) + mul64(r3, s4)
	   + mul64(r4, s3) + mul64(r5, s2) + mul64(r6, s1) + mul64(r7, s0);
	m9 = mul64(r0, s9) + mul64(r1, s8) + mul64(r2, s7) + mul64(r3, s6)
	   + mul64(r4, s5) + mul64(r5, s4) + mul64(r6, s3) + mul64(r7, s2)
	   + mul64(r8, s1) + mul64(r9, s0);

	r1 *= 2;
	r3 *= 2;
	r5 *= 2;
	r7 *= 2;

	m0 = mul64(r0, s0);
	m2 = mul64(r0, s2) + mul64(r1, s1) + mul64(r2, s0);
	m4 = mul64(r0, s4) + mul64(r1, s3) + mul64(r2, s2) + mul64(r3, s1)
	   + mul64(r4, s0);
	m6 = mul64(r0, s6) + mul64(r1, s5) + mul64(r2, s4) + mul64(r3, s3)
	   + mul64(r4, s2) + mul64(r5, s1) + mul64(r6, s0);
	m8 = mul64(r0, s8) + mul64(r1, s7) + mul64(r2, s6) + mul64(r3, s5)
	   + mul64(r4, s4) + mul64(r5, s3) + mul64(r6, s2) + mul64(r7, s1)
	   + mul64(r8, s0);

	r1 *= 19;
	r2 *= 19;
	r3 = (r3 / 2) * 19;
	r4 *= 19;
	r5 = (r5 / 2) * 19;
	r6 *= 19;
	r7 = (r7 / 2) * 19;
	r8 *= 19;
	r9 *= 19;

	m1 += mul64(r9, s2) + mul64(r8, s3) + mul64(r7, s4) + mul64(r6, s5)
	   +  mul64(r5, s6) + mul64(r4, s7) + mul64(r3, s8) + mul64(r2, s9);
	m3 += mul64(r9, s4) + mul64(r8, s5) + mul64(r7, s6) + mul64(r6, s7)
	   +  mul64(r5, s8) + mul64(r4, s9);
	m5 += mul64(r9, s6) + mul64(r8, s7) + mul64(r7, s8) + mul64(r6, s9);
	m7 += mul64(r9, s8) + mul64(r8, s9);

	r3 *= 2;
	r5 *= 2;
	r7 *= 2;
	r9 *= 2;

	m0 += mul64(r9, s1) + mul64(r8, s2) + mul64(r7, s3) + mul64(r6, s4)
	   +  mul64(r5, s5) + mul64(r4, s6) + mul64(r3, s7) + mul64(r2, s8)
	   +  mul64(r1, s9);
	m2 += mul64(r9, s3) + mul64(r8, s4) + mul64(r7, s5) + mul64(r6, s6)
	   +  mul64(r5, s7) + mul64(r4, s8) + mul64(r3, s9);
	m4 += mul64(r9, s5) + mul64(r8, s6) + mul64(r7, s7) + mul64(r6, s8)
	   +  mul64(r5, s9);
	m6 += mul64(r9, s7) + mul64(r8, s8) + mul64(r7, s9);
	m8 += mul64(r9, s9);

	m1 += m0 >> 26; r1 = rdc25(m1);
	m2 += m1 >> 25; r2 = rdc26(m2);
	m3 += m2 >> 26; r3 = rdc25(m3);
	m4 += m3 >> 25; r4 = rdc26(m4);
	m5 += m4 >> 26; r5 = rdc25(m5);
	m6 += m5 >> 25; r6 = rdc26(m6);
	m7 += m6 >> 26; r7 = rdc25(m7);
	m8 += m7 >> 25; r8 = rdc26(m8);
	m9 += m8 >> 26; r9 = rdc25(m9);
	m0 = rdc26(m0) + mul64(m9 >> 25, 19);
	r0 = rdc26(m0); r1 += m0 >> 26;

	out[0] = r0;
	out[1] = r1;
	out[2] = r2;
	out[3] = r3;
	out[4] = r4;
	out[5] = r5;
	out[6] = r6;
	out[7] = r7;
	out[8] = r8;
	out[9] = r9;
}

/**
 * out = in^(2 * count), inlining
 */
static inline void square_times(uint32_t out[10], uint32_t in[10], int count)
{
	uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
	uint32_t d6, d7, d8, d9;
	uint64_t m0, m1, m2, m3, m4, m5, m6, m7, m8, m9;

	r0 = in[0];
	r1 = in[1];
	r2 = in[2];
	r3 = in[3];
	r4 = in[4];
	r5 = in[5];
	r6 = in[6];
	r7 = in[7];
	r8 = in[8];
	r9 = in[9];

	while (count--)
	{
		m0 = mul64(r0, r0    );
		r0 *= 2;
		m1 = mul64(r0, r1    );
		m2 = mul64(r0, r2    ) + mul64(r1, r1 * 2);
		r1 *= 2;
		m3 = mul64(r0, r3    ) + mul64(r1, r2    );
		m4 = mul64(r0, r4    ) + mul64(r1, r3 * 2) + mul64(r2, r2);
		r2 *= 2;
		m5 = mul64(r0, r5    ) + mul64(r1, r4    ) + mul64(r2, r3);
		m6 = mul64(r0, r6    ) + mul64(r1, r5 * 2) + mul64(r2, r4)
		   + mul64(r3, r3 * 2);
		r3 *= 2;
		m7 = mul64(r0, r7    ) + mul64(r1, r6    ) + mul64(r2, r5)
		   + mul64(r3, r4    );
		m8 = mul64(r0, r8    ) + mul64(r1, r7 * 2) + mul64(r2, r6)
		   + mul64(r3, r5 * 2) + mul64(r4, r4    );
		m9 = mul64(r0, r9    ) + mul64(r1, r8    ) + mul64(r2, r7)
		   + mul64(r3, r6    ) + mul64(r4, r5 * 2);

		d6 = r6 * 19;
		d7 = r7 * 2 * 19;
		d8 = r8 * 19;
		d9 = r9 * 2 * 19;

		m0 += mul64(d9, r1    ) + mul64(d8, r2    ) + mul64(d7, r3    )
		   +  mul64(d6, r4 * 2) + mul64(r5, r5 * 2 * 19);
		m1 += mul64(d9, r2 / 2) + mul64(d8, r3    ) + mul64(d7, r4    )
		   +  mul64(d6, r5 * 2);
		m2 += mul64(d9, r3    ) + mul64(d8, r4 * 2) + mul64(d7, r5 * 2)
		   +  mul64(d6, r6    );
		m3 += mul64(d9, r4    ) + mul64(d8, r5 * 2) + mul64(d7, r6    );
		m4 += mul64(d9, r5 * 2) + mul64(d8, r6 * 2) + mul64(d7, r7    );
		m5 += mul64(d9, r6    ) + mul64(d8, r7 * 2);
		m6 += mul64(d9, r7 * 2) + mul64(d8, r8    );
		m7 += mul64(d9, r8    );
		m8 += mul64(d9, r9    );

		m1 += m0 >> 26; r1 = rdc25(m1);
		m2 += m1 >> 25; r2 = rdc26(m2);
		m3 += m2 >> 26; r3 = rdc25(m3);
		m4 += m3 >> 25; r4 = rdc26(m4);
		m5 += m4 >> 26; r5 = rdc25(m5);
		m6 += m5 >> 25; r6 = rdc26(m6);
		m7 += m6 >> 26; r7 = rdc25(m7);
		m8 += m7 >> 25; r8 = rdc26(m8);
		m9 += m8 >> 26; r9 = rdc25(m9);
		m0 = rdc26(m0) + mul64(sr25(m9), 19);
		r0 = rdc26(m0); r1 += sr26(m0);
	}

	out[0] = r0;
	out[1] = r1;
	out[2] = r2;
	out[3] = r3;
	out[4] = r4;
	out[5] = r5;
	out[6] = r6;
	out[7] = r7;
	out[8] = r8;
	out[9] = r9;
}

/**
 * out = in * in
 */
static void square(uint32_t out[10], uint32_t in[10])
{
	return square_times(out, in, 1);
}

/**
 * Take a little-endian, 32-byte number and expand it into polynomial form
 */
static void expand(uint32_t out[10], u_char *in)
{
	uint32_t x0, x1, x2, x3, x4, x5, x6, x7;

	x0 = uletoh32(in + 0);
	x1 = uletoh32(in + 4);
	x2 = uletoh32(in + 8);
	x3 = uletoh32(in + 12);
	x4 = uletoh32(in + 16);
	x5 = uletoh32(in + 20);
	x6 = uletoh32(in + 24);
	x7 = uletoh32(in + 28);

	out[0] = rdc26(                         x0       );
	out[1] = rdc25((((uint64_t)x1 << 32) | x0) >> 26);
	out[2] = rdc26((((uint64_t)x2 << 32) | x1) >> 19);
	out[3] = rdc25((((uint64_t)x3 << 32) | x2) >> 13);
	out[4] = rdc26((                        x3) >>  6);
	out[5] = rdc25(                         x4       );
	out[6] = rdc26((((uint64_t)x5 << 32) | x4) >> 25);
	out[7] = rdc25((((uint64_t)x6 << 32) | x5) >> 19);
	out[8] = rdc26((((uint64_t)x7 << 32) | x6) >> 12);
	out[9] = rdc25((                        x7) >>  6);
}

/**
 * Propagate carries in f
 */
static inline void carry(uint32_t f[10])
{
	f[1] += f[0] >> 26; f[0] = rdc26(f[0]);
	f[2] += f[1] >> 25; f[1] = rdc25(f[1]);
	f[3] += f[2] >> 26; f[2] = rdc26(f[2]);
	f[4] += f[3] >> 25; f[3] = rdc25(f[3]);
	f[5] += f[4] >> 26; f[4] = rdc26(f[4]);
	f[6] += f[5] >> 25; f[5] = rdc25(f[5]);
	f[7] += f[6] >> 26; f[6] = rdc26(f[6]);
	f[8] += f[7] >> 25; f[7] = rdc25(f[7]);
	f[9] += f[8] >> 26; f[8] = rdc26(f[8]);
}

/**
 * Take a fully reduced polynomial form number and contract it into a
 * little-endian, 32-byte array
 */
static void contract(u_char *out, uint32_t f[10])
{
	carry(f);
	f[0] += 19 * (f[9] >> 25); f[9] = rdc25(f[9]);
	carry(f);
	f[0] += 19 * (f[9] >> 25); f[9] = rdc25(f[9]);

	/* now t is between 0 and 2^255-1, properly carried.
	 * case 1: between 0 and 2^255-20.
	 * case 2: between 2^255-19 and 2^255-1.
	 */
	f[0] += 19;
	carry(f);
	f[0] += 19 * (f[9] >> 25); f[9] = rdc25(f[9]);

	/* now between 19 and 2^255-1 in both cases, and offset by 19. */
	f[0] += (1 << 26) - 19;
	f[1] += (1 << 25) - 1;
	f[2] += (1 << 26) - 1;
	f[3] += (1 << 25) - 1;
	f[4] += (1 << 26) - 1;
	f[5] += (1 << 25) - 1;
	f[6] += (1 << 26) - 1;
	f[7] += (1 << 25) - 1;
	f[8] += (1 << 26) - 1;
	f[9] += (1 << 25) - 1;

	/* now between 2^255 and 2^256-20, and offset by 2^255. */
	carry(f);
	f[9] = rdc25(f[9]);

	f[1] <<= 2;
	f[2] <<= 3;
	f[3] <<= 5;
	f[4] <<= 6;
	f[6] <<= 1;
	f[7] <<= 3;
	f[8] <<= 4;
	f[9] <<= 6;

	memset(out, 0, 32);
	horule32(out +  0, f[0]);
	horule32(out +  3, f[1]);
	horule32(out +  6, f[2]);
	horule32(out +  9, f[3]);
	horule32(out + 12, f[4]);
	horule32(out + 16, f[5]);
	horule32(out + 19, f[6]);
	horule32(out + 22, f[7]);
	horule32(out + 25, f[8]);
	horule32(out + 28, f[9]);
}

/**
 * Swap the contents of x and q if swap is non-zero
 */
static void swap_conditional(uint32_t a[10], uint32_t b[10], uint32_t swap)
{
	uint32_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;

	swap = -swap;

	x0 = swap & (a[0] ^ b[0]); a[0] ^= x0; b[0] ^= x0;
	x1 = swap & (a[1] ^ b[1]); a[1] ^= x1; b[1] ^= x1;
	x2 = swap & (a[2] ^ b[2]); a[2] ^= x2; b[2] ^= x2;
	x3 = swap & (a[3] ^ b[3]); a[3] ^= x3; b[3] ^= x3;
	x4 = swap & (a[4] ^ b[4]); a[4] ^= x4; b[4] ^= x4;
	x5 = swap & (a[5] ^ b[5]); a[5] ^= x5; b[5] ^= x5;
	x6 = swap & (a[6] ^ b[6]); a[6] ^= x6; b[6] ^= x6;
	x7 = swap & (a[7] ^ b[7]); a[7] ^= x7; b[7] ^= x7;
	x8 = swap & (a[8] ^ b[8]); a[8] ^= x8; b[8] ^= x8;
	x9 = swap & (a[9] ^ b[9]); a[9] ^= x9; b[9] ^= x9;
}

/*
 * In:  b =   2^5 - 2^0
 * Out: b = 2^250 - 2^0
 */
static void pow_two5mtwo0_two250mtwo0(uint32_t b[10])
{
	uint32_t t0[10], c[10];

	/* 2^5  - 2^0 */ /* b */
	/* 2^10 - 2^5 */ square_times(t0, b, 5);
	/* 2^10 - 2^0 */ mul(b, t0, b);
	/* 2^20 - 2^10 */ square_times(t0, b, 10);
	/* 2^20 - 2^0 */ mul(c, t0, b);
	/* 2^40 - 2^20 */ square_times(t0, c, 20);
	/* 2^40 - 2^0 */ mul(t0, t0, c);
	/* 2^50 - 2^10 */ square_times(t0, t0, 10);
	/* 2^50 - 2^0 */ mul(b, t0, b);
	/* 2^100 - 2^50 */ square_times(t0, b, 50);
	/* 2^100 - 2^0 */ mul(c, t0, b);
	/* 2^200 - 2^100 */ square_times(t0, c, 100);
	/* 2^200 - 2^0 */ mul(t0, t0, c);
	/* 2^250 - 2^50 */ square_times(t0, t0, 50);
	/* 2^250 - 2^0 */ mul(b, t0, b);
}

/*
 * z^(p - 2) = z(2^255 - 21)
 */
static void recip(uint32_t out[10], uint32_t z[10])
{
	uint32_t a[10], t0[10], b[10];

	/* 2 */ square(a, z); /* a = 2 */
	/* 8 */ square_times(t0, a, 2);
	/* 9 */ mul(b, t0, z); /* b = 9 */
	/* 11 */ mul(a, b, a); /* a = 11 */
	/* 22 */ square(t0, a);
	/* 2^5 - 2^0 = 31 */ mul(b, t0, b);
	/* 2^250 - 2^0 */ pow_two5mtwo0_two250mtwo0(b);
	/* 2^255 - 2^5 */ square_times(b, b, 5);
	/* 2^255 - 21 */ mul(out, b, a);
}

METHOD(curve25519_drv_t, curve25519, bool,
	private_curve25519_drv_t *this, u_char *in, u_char *out)
{
	uint32_t nqpqx[10] = {1}, nqpqz[10] = {0}, nqz[10] = {1}, nqx[10];
	uint32_t q[10], qx[10], qpqx[10], qqx[10], zzz[10], zmone[10];
	uint32_t bit, lastbit, i;

	expand(q, in);
	memcpy(nqx, q, sizeof(nqx));

	/* bit 255 is always 0, and bit 254 is always 1, so skip bit 255 and
	 * start pre-swapped on bit 254 */
	lastbit = 1;

	/* we are doing bits 254..3 in the loop, but are swapping in bits 253..2 */
	for (i = 253; i >= 2; i--)
	{
		add(qx, nqx, nqz);
		sub(nqz, nqx, nqz);
		add(qpqx, nqpqx, nqpqz);
		sub(nqpqz, nqpqx, nqpqz);
		mul(nqpqx, qpqx, nqz);
		mul(nqpqz, qx, nqpqz);
		add(qqx, nqpqx, nqpqz);
		sub(nqpqz, nqpqx, nqpqz);
		square(nqpqz, nqpqz);
		square(nqpqx, qqx);
		mul(nqpqz, nqpqz, q);
		square(qx, qx);
		square(nqz, nqz);
		mul(nqx, qx, nqz);
		sub(nqz, qx, nqz);
		scalar_product(zzz, nqz, 121665);
		add(zzz, zzz, qx);
		mul(nqz, nqz, zzz);

		bit = (this->key[i/8] >> (i & 7)) & 1;
		swap_conditional(nqx, nqpqx, bit ^ lastbit);
		swap_conditional(nqz, nqpqz, bit ^ lastbit);
		lastbit = bit;
	}

	/* the final 3 bits are always zero, so we only need to double */
	for (i = 0; i < 3; i++)
	{
		add(qx, nqx, nqz);
		sub(nqz, nqx, nqz);
		square(qx, qx);
		square(nqz, nqz);
		mul(nqx, qx, nqz);
		sub(nqz, qx, nqz);
		scalar_product(zzz, nqz, 121665);
		add(zzz, zzz, qx);
		mul(nqz, nqz, zzz);
	}

	recip(zmone, nqz);
	mul(nqz, nqx, zmone);
	contract(out, nqz);

	return TRUE;
}

METHOD(curve25519_drv_t, destroy, void,
	private_curve25519_drv_t *this)
{
	memwipe(this->key, sizeof(this->key));
	free(this);
}

/**
 * See header
 */
curve25519_drv_t *curve25519_drv_portable_create()
{
	private_curve25519_drv_t *this;

	INIT(this,
		.public = {
			.set_key = _set_key,
			.curve25519 = _curve25519,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
