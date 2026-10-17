// SPDX-License-Identifier: GPL-2.0+ and MIT
/*
 * RSA library - generate parameters for a public key
 *
 * Copyright (c) 2019 Linaro Limited
 * Author: AKASHI Takahiro
 *
 * Big number routines in this file come from BearSSL:
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 */

#include <image.h>
#include <malloc.h>
#include <crypto/internal/rsa.h>
#include <u-boot/rsa-mod-exp.h>
#include <asm/unaligned.h>

/**
 * br_dec16be() - Convert 16-bit big-endian integer to native
 * @src:	Pointer to data
 * Return:	Native-endian integer
 */
static unsigned br_dec16be(const void *src)
{
	return get_unaligned_be16(src);
}

/**
 * br_dec32be() - Convert 32-bit big-endian integer to native
 * @src:	Pointer to data
 * Return:	Native-endian integer
 */
static uint32_t br_dec32be(const void *src)
{
	return get_unaligned_be32(src);
}

/**
 * br_enc32be() - Convert native 32-bit integer to big-endian
 * @dst:	Pointer to buffer to store big-endian integer in
 * @x:		Native 32-bit integer
 */
static void br_enc32be(void *dst, uint32_t x)
{
	__be32 tmp;

	tmp = cpu_to_be32(x);
	memcpy(dst, &tmp, sizeof(tmp));
}

/* from BearSSL's src/inner.h */

/*
 * Negate a boolean.
 */
static uint32_t NOT(uint32_t ctl)
{
	return ctl ^ 1;
}

/*
 * Multiplexer: returns x if ctl == 1, y if ctl == 0.
 */
static uint32_t MUX(uint32_t ctl, uint32_t x, uint32_t y)
{
	return y ^ (-ctl & (x ^ y));
}

/*
 * Equality check: returns 1 if x == y, 0 otherwise.
 */
static uint32_t EQ(uint32_t x, uint32_t y)
{
	uint32_t q;

	q = x ^ y;
	return NOT((q | -q) >> 31);
}

/*
 * Inequality check: returns 1 if x != y, 0 otherwise.
 */
static uint32_t NEQ(uint32_t x, uint32_t y)
{
	uint32_t q;

	q = x ^ y;
	return (q | -q) >> 31;
}

/*
 * Comparison: returns 1 if x > y, 0 otherwise.
 */
static uint32_t GT(uint32_t x, uint32_t y)
{
	/*
	 * If both x < 2^31 and y < 2^31, then y-x will have its high
	 * bit set if x > y, cleared otherwise.
	 *
	 * If either x >= 2^31 or y >= 2^31 (but not both), then the
	 * result is the high bit of x.
	 *
	 * If both x >= 2^31 and y >= 2^31, then we can virtually
	 * subtract 2^31 from both, and we are back to the first case.
	 * Since (y-2^31)-(x-2^31) = y-x, the subtraction is already
	 * fine.
	 */
	uint32_t z;

	z = y - x;
	return (z ^ ((x ^ y) & (x ^ z))) >> 31;
}

/*
 * Compute the bit length of a 32-bit integer. Returned value is between 0
 * and 32 (inclusive).
 */
static uint32_t BIT_LENGTH(uint32_t x)
{
	uint32_t k, c;

	k = NEQ(x, 0);
	c = GT(x, 0xFFFF); x = MUX(c, x >> 16, x); k += c << 4;
	c = GT(x, 0x00FF); x = MUX(c, x >>  8, x); k += c << 3;
	c = GT(x, 0x000F); x = MUX(c, x >>  4, x); k += c << 2;
	c = GT(x, 0x0003); x = MUX(c, x >>  2, x); k += c << 1;
	k += GT(x, 0x0001);
	return k;
}

#define GE(x, y)   NOT(GT(y, x))
#define LT(x, y)   GT(y, x)
#define MUL(x, y)   ((uint64_t)(x) * (uint64_t)(y))

/*
 * Integers 'i32'
 * --------------
 *
 * The 'i32' functions implement computations on big integers using
 * an internal representation as an array of 32-bit integers. For
 * an array x[]:
 *  -- x[0] contains the "announced bit length" of the integer
 *  -- x[1], x[2]... contain the value in little-endian order (x[1]
 *     contains the least significant 32 bits)
 *
 * Multiplications rely on the elementary 32x32->64 multiplication.
 *
 * The announced bit length specifies the number of bits that are
 * significant in the subsequent 32-bit words. Unused bits in the
 * last (most significant) word are set to 0; subsequent words are
 * uninitialized and need not exist at all.
 *
 * The execution time and memory access patterns of all computations
 * depend on the announced bit length, but not on the actual word
 * values. For modular integers, the announced bit length of any integer
 * modulo n is equal to the actual bit length of n; thus, computations
 * on modular integers are "constant-time" (only the modulus length may
 * leak).
 */

/*
 * Extract one word from an integer. The offset is counted in bits.
 * The word MUST entirely fit within the word elements corresponding
 * to the announced bit length of a[].
 */
static uint32_t br_i32_word(const uint32_t *a, uint32_t off)
{
	size_t u;
	unsigned j;

	u = (size_t)(off >> 5) + 1;
	j = (unsigned)off & 31;
	if (j == 0) {
		return a[u];
	} else {
		return (a[u] >> j) | (a[u + 1] << (32 - j));
	}
}

/* from BearSSL's src/int/i32_bitlen.c */

/*
 * Compute the actual bit length of an integer. The argument x should
 * point to the first (least significant) value word of the integer.
 * The len 'xlen' contains the number of 32-bit words to access.
 *
 * CT: value or length of x does not leak.
 */
static uint32_t br_i32_bit_length(uint32_t *x, size_t xlen)
{
	uint32_t tw, twk;

	tw = 0;
	twk = 0;
	while (xlen -- > 0) {
		uint32_t w, c;

		c = EQ(tw, 0);
		w = x[xlen];
		tw = MUX(c, w, tw);
		twk = MUX(c, (uint32_t)xlen, twk);
	}
	return (twk << 5) + BIT_LENGTH(tw);
}

/* from BearSSL's src/int/i32_decode.c */

/*
 * Decode an integer from its big-endian unsigned representation. The
 * "true" bit length of the integer is computed, but all words of x[]
 * corresponding to the full 'len' bytes of the source are set.
 *
 * CT: value or length of x does not leak.
 */
static void br_i32_decode(uint32_t *x, const void *src, size_t len)
{
	const unsigned char *buf;
	size_t u, v;

	buf = src;
	u = len;
	v = 1;
	for (;;) {
		if (u < 4) {
			uint32_t w;

			if (u < 2) {
				if (u == 0) {
					break;
				} else {
					w = buf[0];
				}
			} else {
				if (u == 2) {
					w = br_dec16be(buf);
				} else {
					w = ((uint32_t)buf[0] << 16)
						| br_dec16be(buf + 1);
				}
			}
			x[v ++] = w;
			break;
		} else {
			u -= 4;
			x[v ++] = br_dec32be(buf + u);
		}
	}
	x[0] = br_i32_bit_length(x + 1, v - 1);
}

/* from BearSSL's src/int/i32_encode.c */

/*
 * Encode an integer into its big-endian unsigned representation. The
 * output length in bytes is provided (parameter 'len'); if the length
 * is too short then the integer is appropriately truncated; if it is
 * too long then the extra bytes are set to 0.
 */
static void br_i32_encode(void *dst, size_t len, const uint32_t *x)
{
	unsigned char *buf;
	size_t k;

	buf = dst;

	/*
	 * Compute the announced size of x in bytes; extra bytes are
	 * filled with zeros.
	 */
	k = (x[0] + 7) >> 3;
	while (len > k) {
		*buf ++ = 0;
		len --;
	}

	/*
	 * Now we use k as index within x[]. That index starts at 1;
	 * we initialize it to the topmost complete word, and process
	 * any remaining incomplete word.
	 */
	k = (len + 3) >> 2;
	switch (len & 3) {
	case 3:
		*buf ++ = x[k] >> 16;
		/* fall through */
	case 2:
		*buf ++ = x[k] >> 8;
		/* fall through */
	case 1:
		*buf ++ = x[k];
		k --;
	}

	/*
	 * Encode all complete words.
	 */
	while (k > 0) {
		br_enc32be(buf, x[k]);
		k --;
		buf += 4;
	}
}

/* from BearSSL's src/int/i32_ninv32.c */

/*
 * Compute -(1/x) mod 2^32. If x is even, then this function returns 0.
 */
static uint32_t br_i32_ninv32(uint32_t x)
{
	uint32_t y;

	y = 2 - x;
	y *= 2 - y * x;
	y *= 2 - y * x;
	y *= 2 - y * x;
	y *= 2 - y * x;
	return MUX(x & 1, -y, 0);
}

/* from BearSSL's src/int/i32_add.c */

/*
 * Add b[] to a[] and return the carry (0 or 1). If ctl is 0, then a[]
 * is unmodified, but the carry is still computed and returned. The
 * arrays a[] and b[] MUST have the same announced bit length.
 *
 * a[] and b[] MAY be the same array, but partial overlap is not allowed.
 */
static uint32_t br_i32_add(uint32_t *a, const uint32_t *b, uint32_t ctl)
{
	uint32_t cc;
	size_t u, m;

	cc = 0;
	m = (a[0] + 63) >> 5;
	for (u = 1; u < m; u ++) {
		uint32_t aw, bw, naw;

		aw = a[u];
		bw = b[u];
		naw = aw + bw + cc;

		/*
		 * Carry is 1 if naw < aw. Carry is also 1 if naw == aw
		 * AND the carry was already 1.
		 */
		cc = (cc & EQ(naw, aw)) | LT(naw, aw);
		a[u] = MUX(ctl, naw, aw);
	}
	return cc;
}

/* from BearSSL's src/int/i32_sub.c */

/*
 * Subtract b[] from a[] and return the carry (0 or 1). If ctl is 0,
 * then a[] is unmodified, but the carry is still computed and returned.
 * The arrays a[] and b[] MUST have the same announced bit length.
 *
 * a[] and b[] MAY be the same array, but partial overlap is not allowed.
 */
static uint32_t br_i32_sub(uint32_t *a, const uint32_t *b, uint32_t ctl)
{
	uint32_t cc;
	size_t u, m;

	cc = 0;
	m = (a[0] + 63) >> 5;
	for (u = 1; u < m; u ++) {
		uint32_t aw, bw, naw;

		aw = a[u];
		bw = b[u];
		naw = aw - bw - cc;

		/*
		 * Carry is 1 if naw > aw. Carry is 1 also if naw == aw
		 * AND the carry was already 1.
		 */
		cc = (cc & EQ(naw, aw)) | GT(naw, aw);
		a[u] = MUX(ctl, naw, aw);
	}
	return cc;
}

/* from BearSSL's src/int/i32_div32.c */

/*
 * Constant-time division. The dividend hi:lo is divided by the
 * divisor d; the quotient is returned and the remainder is written
 * in *r. If hi == d, then the quotient does not fit on 32 bits;
 * returned value is thus truncated. If hi > d, returned values are
 * indeterminate.
 */
static uint32_t br_divrem(uint32_t hi, uint32_t lo, uint32_t d, uint32_t *r)
{
	/* TODO: optimize this */
	uint32_t q;
	uint32_t ch, cf;
	int k;

	q = 0;
	ch = EQ(hi, d);
	hi = MUX(ch, 0, hi);
	for (k = 31; k > 0; k --) {
		int j;
		uint32_t w, ctl, hi2, lo2;

		j = 32 - k;
		w = (hi << j) | (lo >> k);
		ctl = GE(w, d) | (hi >> k);
		hi2 = (w - d) >> j;
		lo2 = lo - (d << k);
		hi = MUX(ctl, hi2, hi);
		lo = MUX(ctl, lo2, lo);
		q |= ctl << k;
	}
	cf = GE(lo, d) | hi;
	q |= cf;
	*r = MUX(cf, lo - d, lo);
	return q;
}

/*
 * Wrapper for br_divrem(); the remainder is returned, and the quotient
 * is discarded.
 */
static uint32_t br_rem(uint32_t hi, uint32_t lo, uint32_t d)
{
	uint32_t r;

	br_divrem(hi, lo, d, &r);
	return r;
}

/*
 * Wrapper for br_divrem(); the quotient is returned, and the remainder
 * is discarded.
 */
static uint32_t br_div(uint32_t hi, uint32_t lo, uint32_t d)
{
	uint32_t r;

	return br_divrem(hi, lo, d, &r);
}

/* from BearSSL's src/int/i32_muladd.c */

/*
 * Multiply x[] by 2^32 and then add integer z, modulo m[]. This
 * function assumes that x[] and m[] have the same announced bit
 * length, and the announced bit length of m[] matches its true
 * bit length.
 *
 * x[] and m[] MUST be distinct arrays.
 *
 * CT: only the common announced bit length of x and m leaks, not
 * the values of x, z or m.
 */
static void br_i32_muladd_small(uint32_t *x, uint32_t z, const uint32_t *m)
{
	uint32_t m_bitlen;
	size_t u, mlen;
	uint32_t a0, a1, b0, hi, g, q, tb;
	uint32_t chf, clow, under, over;
	uint64_t cc;

	/*
	 * We can test on the modulus bit length since we accept to
	 * leak that length.
	 */
	m_bitlen = m[0];
	if (m_bitlen == 0) {
		return;
	}
	if (m_bitlen <= 32) {
		x[1] = br_rem(x[1], z, m[1]);
		return;
	}
	mlen = (m_bitlen + 31) >> 5;

	/*
	 * Principle: we estimate the quotient (x*2^32+z)/m by
	 * doing a 64/32 division with the high words.
	 *
	 * Let:
	 *   w = 2^32
	 *   a = (w*a0 + a1) * w^N + a2
	 *   b = b0 * w^N + b2
	 * such that:
	 *   0 <= a0 < w
	 *   0 <= a1 < w
	 *   0 <= a2 < w^N
	 *   w/2 <= b0 < w
	 *   0 <= b2 < w^N
	 *   a < w*b
	 * I.e. the two top words of a are a0:a1, the top word of b is
	 * b0, we ensured that b0 is "full" (high bit set), and a is
	 * such that the quotient q = a/b fits on one word (0 <= q < w).
	 *
	 * If a = b*q + r (with 0 <= r < q), we can estimate q by
	 * doing an Euclidean division on the top words:
	 *   a0*w+a1 = b0*u + v  (with 0 <= v < w)
	 * Then the following holds:
	 *   0 <= u <= w
	 *   u-2 <= q <= u
	 */
	a0 = br_i32_word(x, m_bitlen - 32);
	hi = x[mlen];
	memmove(x + 2, x + 1, (mlen - 1) * sizeof *x);
	x[1] = z;
	a1 = br_i32_word(x, m_bitlen - 32);
	b0 = br_i32_word(m, m_bitlen - 32);

	/*
	 * We estimate a divisor q. If the quotient returned by br_div()
	 * is g:
	 * -- If a0 == b0 then g == 0; we want q = 0xFFFFFFFF.
	 * -- Otherwise:
	 *    -- if g == 0 then we set q = 0;
	 *    -- otherwise, we set q = g - 1.
	 * The properties described above then ensure that the true
	 * quotient is q-1, q or q+1.
	 */
	g = br_div(a0, a1, b0);
	q = MUX(EQ(a0, b0), 0xFFFFFFFF, MUX(EQ(g, 0), 0, g - 1));

	/*
	 * We subtract q*m from x (with the extra high word of value 'hi').
	 * Since q may be off by 1 (in either direction), we may have to
	 * add or subtract m afterwards.
	 *
	 * The 'tb' flag will be true (1) at the end of the loop if the
	 * result is greater than or equal to the modulus (not counting
	 * 'hi' or the carry).
	 */
	cc = 0;
	tb = 1;
	for (u = 1; u <= mlen; u ++) {
		uint32_t mw, zw, xw, nxw;
		uint64_t zl;

		mw = m[u];
		zl = MUL(mw, q) + cc;
		cc = (uint32_t)(zl >> 32);
		zw = (uint32_t)zl;
		xw = x[u];
		nxw = xw - zw;
		cc += (uint64_t)GT(nxw, xw);
		x[u] = nxw;
		tb = MUX(EQ(nxw, mw), tb, GT(nxw, mw));
	}

	/*
	 * If we underestimated q, then either cc < hi (one extra bit
	 * beyond the top array word), or cc == hi and tb is true (no
	 * extra bit, but the result is not lower than the modulus). In
	 * these cases we must subtract m once.
	 *
	 * Otherwise, we may have overestimated, which will show as
	 * cc > hi (thus a negative result). Correction is adding m once.
	 */
	chf = (uint32_t)(cc >> 32);
	clow = (uint32_t)cc;
	over = chf | GT(clow, hi);
	under = ~over & (tb | (~chf & LT(clow, hi)));
	br_i32_add(x, m, over);
	br_i32_sub(x, m, under);
}

/* from BearSSL's src/int/i32_reduce.c */

/*
 * Reduce an integer (a[]) modulo another (m[]). The result is written
 * in x[] and its announced bit length is set to be equal to that of m[].
 *
 * x[] MUST be distinct from a[] and m[].
 *
 * CT: only announced bit lengths leak, not values of x, a or m.
 */
static void br_i32_reduce(uint32_t *x, const uint32_t *a, const uint32_t *m)
{
	uint32_t m_bitlen, a_bitlen;
	size_t mlen, alen, u;

	m_bitlen = m[0];
	mlen = (m_bitlen + 31) >> 5;

	x[0] = m_bitlen;
	if (m_bitlen == 0) {
		return;
	}

	/*
	 * If the source is shorter, then simply copy all words from a[]
	 * and zero out the upper words.
	 */
	a_bitlen = a[0];
	alen = (a_bitlen + 31) >> 5;
	if (a_bitlen < m_bitlen) {
		memcpy(x + 1, a + 1, alen * sizeof *a);
		for (u = alen; u < mlen; u ++) {
			x[u + 1] = 0;
		}
		return;
	}

	/*
	 * The source length is at least equal to that of the modulus.
	 * We must thus copy N-1 words, and input the remaining words
	 * one by one.
	 */
	memcpy(x + 1, a + 2 + (alen - mlen), (mlen - 1) * sizeof *a);
	x[mlen] = 0;
	for (u = 1 + alen - mlen; u > 0; u --) {
		br_i32_muladd_small(x, a[u], m);
	}
}

/**
 * rsa_free_key_prop() - Free key properties
 * @prop:	Pointer to struct key_prop
 *
 * This function frees all the memories allocated by rsa_gen_key_prop().
 */
void rsa_free_key_prop(struct key_prop *prop)
{
	if (!prop)
		return;

	free((void *)prop->modulus);
	free((void *)prop->public_exponent);
	free((void *)prop->rr);

	free(prop);
}

/**
 * rsa_gen_key_prop() - Generate key properties of RSA public key
 * @key:	Specifies key data in DER format
 * @keylen:	Length of @key
 * @prop:	Generated key property
 *
 * This function takes a blob of encoded RSA public key data in DER
 * format, parse it and generate all the relevant properties
 * in key_prop structure.
 * Return a pointer to struct key_prop in @prop on success.
 *
 * Return:	0 on success, negative on error
 */
int rsa_gen_key_prop(const void *key, uint32_t keylen, struct key_prop **prop)
{
	struct rsa_key rsa_key;
	uint32_t *n = NULL, *rr = NULL, *rrtmp = NULL;
	int rlen, i, ret = 0;

	*prop = calloc(sizeof(**prop), 1);
	if (!(*prop)) {
		ret = -ENOMEM;
		goto out;
	}

	ret = rsa_parse_pub_key(&rsa_key, key, keylen);
	if (ret)
		goto out;

	/* modulus */
	/* removing leading 0's */
	for (i = 0; i < rsa_key.n_sz && !rsa_key.n[i]; i++)
		;
	(*prop)->num_bits = (rsa_key.n_sz - i) * 8;
	(*prop)->modulus = malloc(rsa_key.n_sz - i);
	if (!(*prop)->modulus) {
		ret = -ENOMEM;
		goto out;
	}
	memcpy((void *)(*prop)->modulus, &rsa_key.n[i], rsa_key.n_sz - i);

	n = calloc(sizeof(uint32_t), 1 + ((*prop)->num_bits >> 5));
	rr = calloc(sizeof(uint32_t), 1 + (((*prop)->num_bits * 2) >> 5));
	rrtmp = calloc(sizeof(uint32_t), 2 + (((*prop)->num_bits * 2) >> 5));
	if (!n || !rr || !rrtmp) {
		ret = -ENOMEM;
		goto out;
	}

	/* exponent */
	(*prop)->public_exponent = calloc(1, sizeof(uint64_t));
	if (!(*prop)->public_exponent) {
		ret = -ENOMEM;
		goto out;
	}
	memcpy((void *)(*prop)->public_exponent + sizeof(uint64_t)
						- rsa_key.e_sz,
	       rsa_key.e, rsa_key.e_sz);
	(*prop)->exp_len = sizeof(uint64_t);

	/* n0 inverse */
	br_i32_decode(n, &rsa_key.n[i], rsa_key.n_sz - i);
	(*prop)->n0inv = br_i32_ninv32(n[1]);

	/* R^2 mod n; R = 2^(num_bits) */
	rlen = (*prop)->num_bits * 2; /* #bits of R^2 = (2^num_bits)^2 */
	rr[0] = 0;
	*(uint8_t *)&rr[0] = (1 << (rlen % 8));
	for (i = 1; i < (((rlen + 31) >> 5) + 1); i++)
		rr[i] = 0;
	br_i32_decode(rrtmp, rr, ((rlen + 7) >> 3) + 1);
	br_i32_reduce(rr, rrtmp, n);

	rlen = ((*prop)->num_bits + 7) >> 3; /* #bytes of R^2 mod n */
	(*prop)->rr = malloc(rlen);
	if (!(*prop)->rr) {
		ret = -ENOMEM;
		goto out;
	}
	br_i32_encode((void *)(*prop)->rr, rlen, rr);

out:
	free(n);
	free(rr);
	free(rrtmp);
	if (ret < 0)
		rsa_free_key_prop(*prop);
	return ret;
}
