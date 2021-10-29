/*
 * Math component implementation file.
 * General API for calculation purposes.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id:$
 */

#include <bcm_math.h>
#include "bcm_math_defs_priv.h"

#define MAX_CORDIC32_ITER		17 /* maximum number of cordic32 iterations */
#define MAX_CORDIC32_NFRAC		17 /* Number of fractional bits in atan table */
#define CORDIC32_LOG2_PI_OVER_TWO	18 /* LOG2 PI over 2 */
#define CORDIC32_PI_OVER_TWO \
	(1<<CORDIC32_LOG2_PI_OVER_TWO) /* PI/2 as a fixed point number */
#define CORDIC32_PI			 \
	(CORDIC32_PI_OVER_TWO << 1)    /* PI as a fixed point number */
#define CORDIC32_NUM_FRAC_BITS_INTERNAL	29

#define K_LOG2_TOF_RTD_ADJ_WINDOW_LEN	5
#define K_TOF_RTD_ADJ_WINDOW_LEN	(1 << K_LOG2_TOF_RTD_ADJ_WINDOW_LEN)

static int32 atan_tbl[MAX_CORDIC32_ITER] = {
	131072, 77376, 40884, 20753, 10417,
	5213, 2607, 1304, 652, 326, 163, 81, 41, 20, 10, 5, 3
};

/* Look-up table to calculate head room present in a number */
static const uint8 msb_table[] = {
	0, 1, 2, 2, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
};

/* Atan table for cordic >> num2str(atan(1./(2.^[0:17]))/pi*180,8) */
static const math_fixed AtanTbl[] = {
	2949120,
	1740967,
	919879,
	466945,
	234379,
	117304,
	58666,
	29335,
	14668,
	7334,
	3667,
	1833,
	917,
	458,
	229,
	115,
	57,
	29
};

static int16 TWDL_cos_128[129] = {
	256,
	256,
	256,
	256,
	256,
	256,
	255,
	255,
	255,
	254,
	254,
	254,
	253,
	253,
	252,
	252,
	251,
	250,
	250,
	249,
	248,
	248,
	247,
	246,
	245,
	244,
	243,
	242,
	241,
	240,
	239,
	238,
	237,
	235,
	234,
	233,
	231,
	230,
	229,
	227,
	226,
	224,
	223,
	221,
	220,
	218,
	216,
	215,
	213,
	211,
	209,
	207,
	206,
	204,
	202,
	200,
	198,
	196,
	194,
	192,
	190,
	188,
	185,
	183,
	181,
	179,
	177,
	174,
	172,
	170,
	167,
	165,
	162,
	160,
	157,
	155,
	152,
	150,
	147,
	145,
	142,
	140,
	137,
	134,
	132,
	129,
	126,
	123,
	121,
	118,
	115,
	112,
	109,
	107,
	104,
	101,
	98,
	95,
	92,
	89,
	86,
	83,
	80,
	77,
	74,
	71,
	68,
	65,
	62,
	59,
	56,
	53,
	50,
	47,
	44,
	41,
	38,
	34,
	31,
	28,
	25,
	22,
	19,
	16,
	13,
	9,
	6,
	3,
	0,
};

static int16 TWDL_cos[65] = {
	/* cos(2*M_PI*i/256.0) * pow(2.0, TWDL_SFT) */
	256, /* 256.00000000 */
	256, /* 255.92289759 */
	256, /* 255.69163679 */
	255, /* 255.30635691 */
	255, /* 254.76729003 */
	254, /* 254.07476086 */
	253, /* 253.22918655 */
	252, /* 252.23107645 */
	251, /* 251.08103178 */
	250, /* 249.77974529 */
	248, /* 248.32800082 */
	247, /* 246.72667284 */
	245, /* 244.97672595 */
	243, /* 243.07921423 */
	241, /* 241.03528069 */
	239, /* 238.84615650 */
	237, /* 236.51316032 */
	234, /* 234.03769746 */
	231, /* 231.42125904 */
	229, /* 228.66542111 */
	226, /* 225.77184367 */
	223, /* 222.74226972 */
	220, /* 219.57852416 */
	216, /* 216.28251270 */
	213, /* 212.85622075 */
	209, /* 209.30171217 */
	206, /* 205.62112806 */
	202, /* 201.81668547 */
	198, /* 197.89067606 */
	194, /* 193.84546471 */
	190, /* 189.68348809 */
	185, /* 185.40725324 */
	181, /* 181.01933598 */
	177, /* 176.52237945 */
	172, /* 171.91909244 */
	167, /* 167.21224780 */
	162, /* 162.40468075 */
	157, /* 157.49928719 */
	152, /* 152.49902195 */
	147, /* 147.40689700 */
	142, /* 142.22597965 */
	137, /* 136.95939069 */
	132, /* 131.61030251 */
	126, /* 126.18193721 */
	121, /* 120.67756463 */
	115, /* 115.10050039 */
	109, /* 109.45410392 */
	104, /* 103.74177639 */
	98, /* 97.96695869 */
	92, /* 92.13312935 */
	86, /* 86.24380247 */
	80, /* 80.30252554 */
	74, /* 74.31287738 */
	68, /* 68.27846591 */
	62, /* 62.20292606 */
	56, /* 56.08991748 */
	50, /* 49.94312244 */
	44, /* 43.76624352 */
	38, /* 37.56300146 */
	31, /* 31.33713285 */
	25, /* 25.09238792 */
	19, /* 18.83252828 */
	13, /* 12.56132463 */
	6, /* 6.28255450 */
	0, /* 0.00000000 */
};

uint8
math_nbits_32(int32 value)
{
	int32 abs_val;
	uint8 nbits = 0;

	abs_val = ABS(value);
	while ((abs_val >> nbits) > 0) {
		nbits++;
	}

	return nbits;
}

uint32
math_gcd_32(uint32 bigger, uint32 smaller)
{
	uint32 remainder;

	do {
		remainder = bigger % smaller;
		if (remainder) {
			bigger = smaller;
			smaller = remainder;
		} else {
			return smaller;
		}
	} while (TRUE);
}

uint32
math_sqrt_int_32(uint32 value)
{
	uint32 root = 0, shift = 0;

	/* Compute integer nearest to square root of input integer value */
	for (shift = 0; shift < 32; shift += 2) {
		if (((0x40000000 >> shift) + root) <= value) {
			value -= ((0x40000000 >> shift) + root);
			root = (root >> 1) | (0x40000000 >> shift);
		} else {
			root = root >> 1;
		}
	}

	/* round to the nearest integer */
	if (root < value) {
		++root;
	}

	return root;
}

uint8
math_fp_calc_head_room_32(uint32 num)
{
	uint8 msb_pos;

	msb_pos = (num >> 16) ? ((num >> 24) ? (24 + msb_table[(num >> 24) & MASK_8_BITS])
		: (16 + msb_table[(num >> 16) & MASK_8_BITS]))
		: ((num >> 8) ? (8 + msb_table[(num >> 8) & MASK_8_BITS])
		: msb_table[num & MASK_8_BITS]);

	return (32 - msb_pos);
}

uint32
math_qdiv_32(uint32 dividend, uint32 divisor, uint8 precision, bool round)
{
	uint32 quotient, remainder, roundup, rbit;

	/* ASSERT(divisor); */

	quotient = dividend / divisor;
	remainder = dividend % divisor;
	rbit = divisor & 1;
	roundup = (divisor >> 1) + rbit;

	while (precision--) {
		quotient <<= 1;
		if (remainder >= roundup) {
			quotient++;
			remainder = ((remainder - roundup) << 1) + rbit;
		} else {
			remainder <<= 1;
		}
	}

	/* Final rounding */
	if (round && (remainder >= roundup)) {
		quotient++;
	}

	return quotient;
}

uint32
math_qdiv_roundup_32(uint32 dividend, uint32 divisor, uint8 precision)
{
	return math_qdiv_32(dividend, divisor, precision, TRUE);
}

uint32
math_fp_floor_32(uint32 num, uint8 floor_pos)
{
	return num >> floor_pos;
}

uint32
math_fp_round_32(uint32 num, uint8 rnd_pos)
{
	uint32 rnd_val, rnd_out_tmp;

	/* 0.5 in 1.0.rnd_pos */
	rnd_val = 1 << (rnd_pos - 1);
	rnd_val = (rnd_pos == 0) ? 0 : rnd_val;
	rnd_out_tmp = num + rnd_val;

	return (rnd_out_tmp >> rnd_pos);
}

void
math_add_64(uint32 *r_hi, uint32 *r_lo, uint32 offset)
{
	uint32 r1_lo = *r_lo;
	(*r_lo) += offset;
	if (*r_lo < r1_lo) {
		(*r_hi)++;
	}
}

void
math_sub_64(uint32 *r_hi, uint32 *r_lo, uint32 offset)
{
	uint32 r1_lo = *r_lo;
	(*r_lo) -= offset;
	if (*r_lo > r1_lo) {
		(*r_hi)--;
	}
}

void
math_uint64_multiple_add(uint32 *r_high, uint32 *r_low, uint32 a, uint32 b, uint32 c)
{
#define FORMALIZE(var) {cc += (var & 0x80000000) ? 1 : 0; var &= 0x7fffffff;}
	uint32 r1, r0;
	uint32 a1, a0, b1, b0, t, cc = 0;

	a1 = a >> 16;
	a0 = a & 0xffff;
	b1 = b >> 16;
	b0 = b & 0xffff;

	r0 = a0 * b0;
	FORMALIZE(r0);

	t = (a1 * b0) << 16;
	FORMALIZE(t);

	r0 += t;
	FORMALIZE(r0);

	t = (a0 * b1) << 16;
	FORMALIZE(t);

	r0 += t;
	FORMALIZE(r0);

	FORMALIZE(c);

	r0 += c;
	FORMALIZE(r0);

	r0 |= (cc % 2) ? 0x80000000 : 0;
	r1 = a1 * b1 + ((a1 * b0) >> 16) + ((b1 * a0) >> 16) + (cc / 2);

	*r_high = r1;
	*r_low = r0;
}

void
math_uint64_divide(uint32 *r, uint32 a_high, uint32 a_low, uint32 b)
{
	uint32 a1 = a_high, a0 = a_low, r0 = 0;

	if (b < 2) {
		return;
	}

	while (a1 != 0) {
		r0 += (0xffffffff / b) * a1;
		math_uint64_multiple_add(&a1, &a0, ((0xffffffff % b) + 1) % b, a1, a0);
	}

	r0 += a0 / b;
	*r = r0;
}

uint64
math_div_64(uint32 a_high, uint32 a_low, uint32 b)
{
	uint32 a1 = a_high, a0 = a_low;
	uint64 r0 = 0;

	if (!b) {
		/* ASSERT(0); */
		return 0;
	}
	if (b == 1) {
		return (((uint64)a1 << 32) | a0);
	}

	while (a1 != 0) {
		r0 += (uint64)(0xffffffff / b) * a1;
		math_uint64_multiple_add(&a1, &a0, ((0xffffffff % b) + 1) % b, a1, a0);
	}

	r0 += a0 / b;
	return r0;
}

void
math_uint64_right_shift(uint32 *r, uint32 a_high, uint32 a_low, uint32 b)
{
	uint32 a1 = a_high, a0 = a_low, r0 = 0;

	if (b == 0) {
		r0 = a_low;
		*r = r0;
		return;
	}

	if (b < 32) {
		a0 = a0 >> b;
		a1 = a1 & ((1 << b) - 1);
		a1 = a1 << (32 - b);
		r0 = a0 | a1;
		*r = r0;
		return;
	} else {
		r0 = a1 >> (b - 32);
		*r = r0;
		return;
	}
}

uint64
math_shl_64(uint64 num, uint8 shift_amt)
{
	uint32 in_hi, in_lo;
	uint32 masked_lo = 0;
	uint32 mask;
	uint64 shl_out;

	if (shift_amt == 0) {
		return num;
	}

	/* Get hi and lo part */
	in_hi = (uint32)((uint64)num >> 32) & MASK_32_BITS;
	in_lo = (uint32)(num & MASK_32_BITS);

	if (shift_amt < 32) {
		/* Extract bit which belongs to hi part after shifting */
		mask = ((uint32)~0) << (32 - shift_amt);
		masked_lo = (in_lo & mask) >> (32 - shift_amt);

		/* Shift hi and lo and prepare output */
		in_hi = (in_hi << shift_amt) | masked_lo;
		in_lo = in_lo << shift_amt;
	} else {
		/* Extract bit which belongs to hi part after shifting */
		shift_amt = shift_amt - 32;

		/* Shift hi and lo and prepare output */
		in_hi = in_lo << shift_amt;
		in_lo = 0;
	}

	shl_out = (((uint64)in_hi << 32) | in_lo);
	return shl_out;
}

uint64
math_shr_64(uint64 num, uint8 shift_amt)
{
	uint32 in_hi, in_lo;
	uint32 masked_hi = 0;
	uint32 mask;
	uint64 shr_out;

	if (shift_amt == 0) {
		return num;
	}

	/* Get hi and lo part */
	in_hi = (uint32)((uint64)num >> 32) & MASK_32_BITS;
	in_lo = (uint32)(num & MASK_32_BITS);

	if (shift_amt < 32) {
		/* Extract bit which belongs to lo part after shifting */
		mask = (1 << shift_amt) - 1;
		masked_hi = in_hi & mask;

		/* Shift hi and lo and prepare output */
		in_hi = (uint32)in_hi >> shift_amt;
		in_lo = ((uint32)in_lo >> shift_amt) | (masked_hi << (32 - shift_amt));
	} else {
		shift_amt = shift_amt - 32;
		in_lo = in_hi >> shift_amt;
		in_hi = 0;
	}

	shr_out = (((uint64)in_hi << 32) | in_lo);
	return shr_out;
}

uint64
math_fp_mult_64(uint64 val1, uint64 val2, uint8 nf1, uint8 nf2, uint8 nf_res)
{
	uint64 mult_out_tmp, mult_out, rnd_val;
	uint8 shift_amt;

	shift_amt = nf1 + nf2 - nf_res;
	/* 0.5 in 1.0.shift_amt */
	rnd_val = math_shl_64(1, (shift_amt - 1));
	rnd_val = (shift_amt == 0) ? 0 : rnd_val;
	mult_out_tmp = (uint64)((uint64)val1 * (uint64)val2) + (uint64)rnd_val;
	mult_out = math_shr_64(mult_out_tmp, shift_amt);

	return mult_out;
}

uint8
math_fp_div_64(uint64 num, uint32 den, uint8 nf_num, uint8 nf_den, uint32 *div_out)
{
	uint8 shift_amt1, shift_amt2, shift_amt, nf_res, hd_rm_nr, hd_rm_dr;
	uint32 num_hi, num_lo;
	uint64 num_scale;

	/* Worst case shift possible */
	hd_rm_nr = math_fp_calc_head_room_64(num);
	hd_rm_dr = math_fp_calc_head_room_32(den);

	/* (Nr / Dr) <= 2^32 */
	shift_amt1 = hd_rm_nr - hd_rm_dr - 1;
	/* Shift <= 32 + N2 - N1 */
	shift_amt2 = 31 + nf_den - nf_num;
	shift_amt = MINIMUM(shift_amt1, shift_amt2);

	/* Scale numerator */
	num_scale = math_shl_64(num, shift_amt);

	/* Do division */
	num_hi = (uint32)((uint64)num_scale >> 32) & MASK_32_BITS;
	num_lo = (uint32)(num_scale & MASK_32_BITS);
	math_uint64_divide(div_out, num_hi, num_lo, den);

	/* Result format */
	nf_res = nf_num - nf_den + shift_amt;
	return nf_res;
}

uint8
math_fp_calc_head_room_64(uint64 num)
{
	uint8 n_room_bits = 0, msb_pos;
	uint32 num_hi, num_lo, x;

	num_hi = (uint32)((uint64)num >> 32) & MASK_32_BITS;
	num_lo = (uint32)(num & MASK_32_BITS);

	if (num_hi > 0) {
		x = num_hi;
		n_room_bits = 0;
	} else {
		x = num_lo;
		n_room_bits = 32;
	}

	msb_pos = (x >> 16) ? ((x >> 24) ? (24 + msb_table[(x >> 24) & MASK_8_BITS])
		: (16 + msb_table[(x >> 16) & MASK_8_BITS]))
		: ((x >> 8) ? (8 + msb_table[(x >> 8) & MASK_8_BITS])
		: msb_table[x & MASK_8_BITS]);

	return (n_room_bits + 32 - msb_pos);
}

uint32
math_fp_floor_64(uint64 num, uint8 floor_pos)
{
	uint32 floor_out;

	floor_out = (uint32)math_shr_64(num, floor_pos);

	return floor_out;
}

uint32
math_fp_round_64(uint64 num, uint8 rnd_pos)
{
	uint64 rnd_val, rnd_out_tmp;
	uint32 rnd_out;

	/* 0.5 in 1.0.rnd_pos */
	rnd_val = math_shl_64(1, (rnd_pos - 1));
	rnd_val = (rnd_pos == 0) ? 0 : rnd_val;
	rnd_out_tmp = num + rnd_val;
	rnd_out = (uint32)math_shr_64(rnd_out_tmp, rnd_pos);

	return rnd_out;
}

uint32
math_fp_ceil_64(uint64 num, uint8 ceil_pos)
{
	uint64 ceil_val, ceil_out_tmp;
	uint32 ceil_out;

	/* 0.999 in 1.0.rnd_pos */
	ceil_val = math_shl_64(1, ceil_pos) - 1;
	ceil_out_tmp = num + ceil_val;
	ceil_out = (uint32)math_shr_64(ceil_out_tmp, ceil_pos);

	return ceil_out;
}

void
math_cmplx_add_cint32(const cint32 *in1, const cint32 *in2, cint32 *out)
{
	out->i = in1->i + in2->i;
	out->q = in1->q + in2->q;
}

void
math_cmplx_power_cint32(const cint32 *in1, uint32 *pwr)
{
	int32 re = in1->i;
	int32 im = in1->q;
	*(pwr) = (re * re + im * im);
}

void
math_cmplx_mult_cint32_cfixed(const cint32 *in1, const cint32 *in2, const uint8 prec,
	cint32 *out, bool conj)
{
	int32 i1 = in1->i, q1 = in1->q;
	int32 i2 = in2->i, q2 = in2->q * (conj ? -1 : 1);
	out->i = (i1 * i2 - q1 * q2) / (1 << prec);
	out->q = (i1 * q2 + i2 * q1) / (1 << prec);
}

void
math_cmplx_power_cint32_arr(const cint32 *in1, const uint16 *idx_arr, uint16 len, uint32 *pwr)
{
	uint32 tmp_pwr = 0;
	uint16 i = 0, idx;
	*pwr = 0;

	for (i = 0; i < len; i++) {
		idx = *(idx_arr + i);
		math_cmplx_power_cint32((in1 + idx), &tmp_pwr);
		*(pwr) += tmp_pwr;
	}
}

void
math_cmplx_cordic(math_fixed theta, math_cint32 *val)
{
	math_fixed angle, valtmp;
	unsigned iter;
	int signx = 1;
	int signtheta;

	val[0].i = CORDIC_AG;
	val[0].q = 0;
	angle = 0;

	/* limit angle to -180 .. 180 */
	signtheta = (theta < 0) ? -1 : 1;
	theta = ((theta + FIXED(180) * signtheta) % FIXED(360)) - FIXED(180) * signtheta;

	/* rotate if not in quadrant one or four */
	if (FLOAT(theta) > 90) {
		theta -= FIXED(180);
		signx = -1;
	} else if (FLOAT(theta) < -90) {
		theta += FIXED(180);
		signx = -1;
	}

	/* run cordic iterations */
	for (iter = 0; iter < CORDIC_NI; iter++) {
		if (theta > angle) {
			valtmp = val[0].i - (val[0].q >> iter);
			val[0].q = (val[0].i >> iter) + val[0].q;
			val[0].i = valtmp;
			angle += AtanTbl[iter];
		} else {
			valtmp = val[0].i + (val[0].q >> iter);
			val[0].q = -(val[0].i >> iter) + val[0].q;
			val[0].i = valtmp;
			angle -= AtanTbl[iter];
		}
	}

	/* re-rotate quadrant two and three points */
	val[0].i = val[0].i * signx;
	val[0].q = val[0].q * signx;
}

void
math_cmplx_invcordic(math_cint32 val, int32 *angle)
{
	int valtmp;
	unsigned iter;

	*angle = 0;
	val.i = val.i << 4;
	val.q = val.q << 4;

	/* run cordic iterations */
	for (iter = 0; iter < CORDIC_NI; iter++) {
		if (val.q < 0) {
			valtmp = val.i - (val.q >> iter);
			val.q = (val.i >> iter) + val.q;
			val.i = valtmp;
			*angle -= AtanTbl[iter];
		} else {
			valtmp = val.i + (val.q >> iter);
			val.q = -(val.i >> iter) + val.q;
			val.i = valtmp;
			*angle += AtanTbl[iter];
		}
	}
}

void
math_cmplx_computedB(uint32 *cmplx_pwr, int8 *p_cmplx_pwr_dB, uint8 core)
{
	uint8 shift_ct, lsb, msb, secondmsb, thirdmsb, i;
	uint32 tmp;

	for (i = 0; i < core; i++) {
		tmp = cmplx_pwr[i];
		shift_ct = msb = secondmsb = thirdmsb = 0;

		while (tmp != 0) {
			tmp = tmp >> 1;
			shift_ct++;
			lsb = (uint8)(tmp & 1);
			if (lsb == 1) {
				msb = shift_ct;
			}
		}

		if (msb != 0) {
			secondmsb = (uint8)((cmplx_pwr[i] >> (msb - 1)) & 1);
			thirdmsb = (uint8)((cmplx_pwr[i] >> (msb - 2)) & 1);
		}

		p_cmplx_pwr_dB[i] = (int8)(3 * msb + 2 * secondmsb + thirdmsb);
	}
}

void
math_cmplx_angle_to_phasor_lut(uint16 angle, uint16 *packed_word)
{
	int16 sin_tbl[] = {
		0x000, 0x00d, 0x019, 0x026, 0x032, 0x03f, 0x04b, 0x058,
		0x064, 0x070, 0x07c, 0x089, 0x095, 0x0a1, 0x0ac, 0x0b8,
		0x0c4, 0x0cf, 0x0db, 0x0e6, 0x0f1, 0x0fc, 0x107, 0x112,
		0x11c, 0x127, 0x131, 0x13b, 0x145, 0x14e, 0x158, 0x161,
		0x16a, 0x173, 0x17b, 0x184, 0x18c, 0x194, 0x19b, 0x1a3,
		0x1aa, 0x1b1, 0x1b7, 0x1bd, 0x1c4, 0x1c9, 0x1cf, 0x1d4,
		0x1d9, 0x1de, 0x1e2, 0x1e6, 0x1ea, 0x1ed, 0x1f1, 0x1f4,
		0x1f6, 0x1f8, 0x1fa, 0x1fc, 0x1fe, 0x1ff, 0x1ff, 0x200
	};

	uint16 k, num_angles = 2;
	uint16 theta[2], recip_coef_nfrac = 11;
	int16  re = 0, im = 0, exp, quad;
	int16  sin_idx, cos_idx;
	int16  sin_out, cos_out;
	uint32 packed;

	theta[0] = (uint8)(angle & 0xFF);
	theta[1] = (uint8)((angle >> 8) & 0xFF);

	/* printf("---- theta1 = %d, theta2 = %d\n", theta[0], theta[1]); */

	for (k = 0, packed = 0; k < num_angles; k++) {

		/* 6 LSBs for 1st quadrant */
		sin_idx = (theta[k] & 0x3F);
		cos_idx = 63 - sin_idx;

		sin_out = sin_tbl[sin_idx];
		cos_out = sin_tbl[cos_idx];

		/* 2MSBs for quadrant */
		quad = ((theta[k] >> 6) & 0x3);

		if (quad == 0) {
			re = cos_out; im = -sin_out;
		} else if (quad == 1) {
			re = -sin_out; im = -cos_out;
		} else if (quad == 2) {
			re = -cos_out; im = sin_out;
		} else if (quad == 3) {
			re = sin_out; im = cos_out;
		}

		re += (re < 0) ? (1 << recip_coef_nfrac) : 0;
		im += (im < 0) ? (1 << recip_coef_nfrac) : 0;
		exp = 1;

		packed = (uint32)((exp << (2 * recip_coef_nfrac)) |
			(im << recip_coef_nfrac) | re);

		if (k == 0) {
			packed_word[0] = (packed & 0xFFFF);
			packed_word[1] = (packed >> 16) & 0xFF;
		} else if (k == 1) {
			packed_word[1] |= ((packed & 0xFF) << 8);
			packed_word[2] = (packed >> 8) & 0xFFFF;
		}
	}
	/* printf("reciprocity packed_word: %x%x%x\n",
	packed_word[2], packed_word[1], packed_word[0]);
	*/
}

void
math_mat_rho(int64 *n, int64 *p, int64 *rho, int m)
{
	int i;
	int q1 = 2;

	for (i = 0; i < m; i++) {
		*(rho + (i * 3) + 0) = 1;
		*(rho + (i * 3) + 1) = *(n + (i * 1) + 0);
		*(rho + (i * 3) + 2) =
			-(*(n + (i * 1) + 0) * (*(p + (i * 1) + 0)));
		*(rho + (i * 3) + 2) = (*(rho + (i * 3) + 2) + (int64)(1 << (q1 - 1))) >> q1;
	}
}

void
math_mat_transpose(int64 *a, int64 *b, int m, int n)
{
	int i, j;

	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++) {
			/* b[j][i] = a[i][j]; */
			*(b + (j * m) + i) = *(a + (i * n) + j);
		}
	}
}

void
math_mat_mult(int64 *a, int64 *b, int64 *c, int m, int n, int r)
{
	int i, j, k;

	for (i = 0; i < m; i++) {
		for (j = 0; j < r; j++) {
			*(c + (i * r) + j) = 0;
			for (k = 0; k < n; k++) {
				/* c[i][j] += a[i][k] * b[k][j]; */
				*(c + (i * r) + j) += *(a + (i * n) + k) * *(b + (k * r) + j);
			}
		}
	}
}

void
math_mat_inv_prod_det(int64 *a, int64 *b)
{
	/* C2_calc = [	a22*a33 - a32*a23  a13*a32 - a12*a33  a12*a23 - a13*a22
	a23*a31 - a21*a33  a11*a33 - a13*a31  a13*a21 - a11*a23
	a21*a32 - a31*a22  a12*a31 - a11*a32  a11*a22 - a12*a21];
	*/

	int64 a11 = *(a + (0 * 3) + 0);
	int64 a12 = *(a + (0 * 3) + 1);
	int64 a13 = *(a + (0 * 3) + 2);

	int64 a21 = *(a + (1 * 3) + 0);
	int64 a22 = *(a + (1 * 3) + 1);
	int64 a23 = *(a + (1 * 3) + 2);

	int64 a31 = *(a + (2 * 3) + 0);
	int64 a32 = *(a + (2 * 3) + 1);
	int64 a33 = *(a + (2 * 3) + 2);

	*(b + (0 * 3) + 0) = a22 * a33 - a32 * a23;
	*(b + (0 * 3) + 1) = a13 * a32 - a12 * a33;
	*(b + (0 * 3) + 2) = a12 * a23 - a13 * a22;

	*(b + (1 * 3) + 0) = a23 * a31 - a21 * a33;
	*(b + (1 * 3) + 1) = a11 * a33 - a13 * a31;
	*(b + (1 * 3) + 2) = a13 * a21 - a11 * a23;

	*(b + (2 * 3) + 0) = a21 * a32 - a31 * a22;
	*(b + (2 * 3) + 1) = a12 * a31 - a11 * a32;
	*(b + (2 * 3) + 2) = a11 * a22 - a12 * a21;
}

void
math_mat_det(int64 *a, int64 *det)
{
	/* det_C1 = a11*a22*a33 + a12*a23*a31 + a13*a21*a32 -
	a11*a23*a32 - a12*a21*a33 - a13*a22*a31;
	*/

	int64 a11 = *(a + (0 * 3) + 0);
	int64 a12 = *(a + (0 * 3) + 1);
	int64 a13 = *(a + (0 * 3) + 2);

	int64 a21 = *(a + (1 * 3) + 0);
	int64 a22 = *(a + (1 * 3) + 1);
	int64 a23 = *(a + (1 * 3) + 2);

	int64 a31 = *(a + (2 * 3) + 0);
	int64 a32 = *(a + (2 * 3) + 1);
	int64 a33 = *(a + (2 * 3) + 2);

	*det = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 -
		a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31;
}

int32 math_fft_cos_seq20(int i)
{
	int32 res;

	if (i < 65) {
		res = TWDL_cos_128[i * 2];
	} else if (i < 129) {
		res = -1 * TWDL_cos_128[256 - i * 2];
	} else if (i < 193) {
		res = -1 * TWDL_cos_128[i * 2 - 256];
	} else {
		res = TWDL_cos_128[512 - i * 2];
	}

	return res;
}

int32 math_fft_cos_128(int i)
{
	int32 res;

	if (i < 129) {
		res = TWDL_cos_128[i];
	} else if (i < 257) {
		res = -1 * TWDL_cos_128[256 - i];
	} else if (i < 385) {
		res = -1 * TWDL_cos_128[i - 256];
	} else {
		res = TWDL_cos_128[512 - i];
	}

	return res;
}

int32 math_fft_sin_seq20(int i)
{
	int32 res;

	if (i < 65) {
		res = -1 * TWDL_cos_128[128 - i * 2];
	} else if (i < 129) {
		res = -1 * TWDL_cos_128[i * 2 - 128];
	} else if (i < 193) {
		res = TWDL_cos_128[384 - i * 2];
	} else {
		res = TWDL_cos_128[i * 2 - 384];
	}

	return res;
}

int32 math_fft_sin_128(int i)
{
	int32 res;

	if (i < 129) {
		res = -1 * TWDL_cos_128[128 - i];
	} else if (i < 257) {
		res = -1 * TWDL_cos_128[i - 128];
	} else if (i < 385) {
		res = TWDL_cos_128[384 - i];
	} else {
		res = TWDL_cos_128[i - 384];
	}

	return res;
}

int32 math_fft_cos(int i)
{
	int32 res;

	if (i < 65) {
		res = TWDL_cos[i];
	} else if (i < 129) {
		res = -1 * TWDL_cos[128 - i];
	} else if (i < 193) {
		res = -1 * TWDL_cos[i - 128];
	} else {
		res = TWDL_cos[256 - i];
	}

	return res;
}

int32 math_fft_sin(int i)
{
	int32 res;

	if (i < 65) {
		res = -1 * TWDL_cos[64 - i];
	} else if (i < 129) {
		res = -1 * TWDL_cos[i - 64];
	} else if (i < 193) {
		res = TWDL_cos[192 - i];
	} else {
		res = TWDL_cos[i - 192];
	}

	return res;
}

/* Cordic calculation */
int32
math_cordic(cint32 phasor)
{
	int32 x, y, z, prev_x, prev_y, prev_z, mu;
	int iter, signx, signy;

	x = phasor.i;
	y = phasor.q;
	z = 0;

	/* Figure out in which quadrant we are */
	signx = (x < 0)? -1 : 1;
	signy = (y <= 0)? -1 : 1;

	/* If x < 0, negate x and y (rotate 180 degrees) */
	/* This rotates into the 1st or 4th quadrant */
	/* CORDIC only works in 1st and 4th quadrant */
	x = signx * x;
	y = signx * y;

	/* CORDIC iteration */
	iter = 0;
	while (iter < MAX_CORDIC32_ITER) {
		prev_x = x;
		prev_y = y;
		prev_z = z;
		mu = (y < 0)? 1 : -1;
		z = prev_z - mu * atan_tbl[iter];
		x = prev_x - mu * ROUND(prev_y, iter);
		y = prev_y + mu * ROUND(prev_x, iter);
		x = LIMIT(x, -(1<<CORDIC32_NUM_FRAC_BITS_INTERNAL),
			(1<<CORDIC32_NUM_FRAC_BITS_INTERNAL)-1);
		y = LIMIT(y, -(1<<CORDIC32_NUM_FRAC_BITS_INTERNAL),
			(1<<CORDIC32_NUM_FRAC_BITS_INTERNAL)-1);
		iter++;
	}

	/* If in 2nd quadrant, output angle is (z + pi). */
	/* If in 3rd quadrant, output angle is (z - pi). */
	/* Otherwise, output angle is z. */
	if (signx < 0) {
		z = z + CORDIC32_PI * signy;
	}

	/* Limit angle to [-pi, pi) */
	z = LIMIT(z, -CORDIC32_PI, CORDIC32_PI);

	return z;
}

int32
math_cordic_ptr(void *value)
{
	cint32 phasor = *((cint32*)value);
	return math_cordic(phasor);
}

static int32
math_theta_to_idx(int32 theta)
{
	int32 tmp;
	tmp = theta / (2 * CORDIC32_PI);
	theta = theta - ((int)tmp) * 2 * CORDIC32_PI;
	theta = (theta < 0) ? (2 * CORDIC32_PI + theta) : theta;
	tmp = (theta * 512) / (2 * CORDIC32_PI);
	return tmp;
}

int32
math_cos_tbl(int32 theta)
{
	int32 i = math_theta_to_idx(theta);
	return math_fft_cos_128(i);
}

int32
math_sin_tbl(int32 theta)
{
	int32 i = math_theta_to_idx(-theta);
	return math_fft_sin_128(i);
}
